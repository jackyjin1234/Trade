#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <iostream>
#include <getopt.h>
#include <fstream>
#include <sstream>

// IDENTIFIER  = 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98

using namespace std;

class Bank {
private:
    struct transact {
        int64_t timestamp = 0;
        int64_t ip = 0;
        string sender;
        string recipient;
        int amount = 0;
        int64_t execTime = 0;
        bool os = false;
        int id = 0;
        int fee = 0;
    };

    struct user {
        int64_t pin = 0;
        unordered_set<int64_t> ip;
        int balance = 0;
        int64_t regTime = 0;
        std::vector<transact> incoming;
        std::vector<transact> outgoing;
    };

    struct TransactCompare {
        bool operator()(const transact& a, const transact& b) {
            if (a.execTime == b.execTime) {
                return a.id > b.id;
            }
            return a.execTime > b.execTime;
        }
    };

    // Member variables
    std::unordered_map<std::string, user> users;
    std::priority_queue<transact, std::vector<transact>, TransactCompare> todo;
    std::vector<transact> executed;
    int64_t time;

public:
    bool verbose;

    void registration(const std::string& filename);
    void commands();
    void execute();
    void query();
    int64_t readIP(string a);
    int64_t readTime(string a);
    int tradeCount = 0;
};

void Bank::registration(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        user temp;
        std::string input, userId;
        
        std::getline(iss, input, '|');
        temp.regTime = readTime(input);

        std::getline(iss, userId, '|');
        std::getline(iss, input, '|');
        temp.pin = std::stoi(input);
        std::getline(iss, input);
        temp.balance = std::stoi(input);
        
        // cout << "Debug Info: regTime = " << temp.regTime << ", userId = " << userId << ", pin = " << temp.pin << ", balance = " << temp.balance << '\n';

        users[userId] = temp;
    }

    file.close();
}

void Bank::commands() {
    std::string field;
    char firstChar;
    while (cin >> firstChar) {
        if (cin.fail()) {
            cerr << "Error: Reading from cin has failed" << endl;
            exit(1);
        }
        if (firstChar == '#') {
            getline(cin, field);
            // cout << "in#\n";
        } else if (firstChar == 'l') {
            // cout << "inl\n";
            string id, pin, ip;
            cin >> field >> id >> pin >> ip;
            // cout << "Debug Info: id = " << id << ", pin = " << pin << ", ip = " << ip << '\n';
            auto temp = users.find(id);
            if (temp == users.end() || (temp != users.end() && temp->second.pin != stoi(pin))) {
                if (verbose) {
                    cout << "Login failed for " << id << '.' << '\n';
                }
            } else {
                temp->second.ip.insert(readIP(ip));
                if (verbose) {
                    cout << "User " << id << " logged in." << '\n';
                }
            }
        } else if (firstChar == 'o') {
            // cout << "ino\n";
            string id, ip;
            cin >> field >> id >> ip;
            int64_t ip_ = readIP(ip);
            auto temp = users.find(id);
            if (temp == users.end() || (temp != users.end() && temp->second.ip.find(ip_) == temp->second.ip.end())) {
                if (verbose) {
                    cout << "Logout failed for " << id << '.' << '\n';
                }
            } else {
                temp->second.ip.erase(ip_);
                if (verbose) {
                    cout << "User " << id << " logged out." << '\n';
                }
            }
        } else if (firstChar == 'b') {
            // cout << "inb\n";
            int64_t temp_time = time;

            string id, ip;
            cin >> field >> id >> ip;
            auto temp = users.find(id);

            if (temp_time == 0) {
                temp_time = temp->second.regTime;
            }

            if (temp == users.end()) {
                if (verbose) {                      // do we need to account for regTime?
                    cout << "Account " << id << " does not exist." << '\n';
                }
            } else if (temp->second.ip.empty()) {
                if (verbose) {
                    cout << "Account " << id << " is not logged in." << '\n';
                }
            } else if (temp->second.ip.find(readIP(ip)) == temp->second.ip.end()) {
                if (verbose) {
                    cout << "Fraudulent transaction detected, aborting request." << '\n';
                }
            } else {
                cout << "As of " << temp_time << ", " << id << " has a balance of $" << temp->second.balance << '.' << '\n';
            }
        } else if (firstChar == 'p') {
            // cout << "inp\n";
            string timestamp, ip, sender, recipient, amount, exedate, os;
            bool os_ = false;
            cin >> field >> timestamp >> ip >> sender >> recipient >> amount >> exedate >> os;
            // cout << "Debug: timestamp=" << timestamp << ", ip=" << ip << ", sender=" << sender << ", recipient=" << recipient << ", amount=" << amount << ", exedate=" << exedate << ", os=" << os << '\n';
            int64_t timestamp_ = readTime(timestamp);
            int64_t ip_ = readIP(ip);
            int64_t exedate_ = readTime(exedate);
            if (os == "s") {
                os_ = true;
            }
            auto temp1 = users.find(sender);
            auto temp2 = users.find(recipient);

            // cout << "finished read\n";

            if (timestamp_ < time) {
                cerr << "timestamp in place command is less than current time\n";
                exit(1);
            }
            if (exedate_ < timestamp_) {
                cerr << "execution before timestamp\n";
                exit(1);
            }

            time = timestamp_;

            // cout << "successed placed\n";
            

            if (sender == recipient) {
                if (verbose) {
                    cout << "Self transactions are not allowed." << '\n';
                }
                continue;
            } else if (timestamp_ + 3000000 < exedate_) {
                if (verbose) {
                    cout << "Select a time up to three days in the future.\n";
                }
                continue;
            } else if (temp1 == users.end()) {
                if (verbose) {
                    cout << "Sender " << sender << " does not exist.\n";
                }
                continue;
            } else if (temp2 == users.end()) {
                if (verbose) {
                    cout << "Recipient " << recipient << " does not exist.\n";
                }
                continue;
            } else if (temp1->second.regTime > exedate_ || temp2->second.regTime > exedate_) {
                if (verbose) {
                    cout << "At the time of execution, sender and/or recipient have not registered.\n";
                }
                continue;
            } else if (temp1->second.ip.empty()) {
                if (verbose) {
                    cout << "Sender " << sender << " is not logged in.\n";
                }
                continue;
            } else if (temp1->second.ip.find(ip_) == temp1->second.ip.end()) {
                if (verbose) {
                    cout << "Fraudulent transaction detected, aborting request." << '\n';
                }
                continue;
            } else {
                while (!todo.empty() && todo.top().execTime <= timestamp_) {
                    execute();
                }
                if (verbose) {
                    cout << "Transaction " << tradeCount << " placed at " << timestamp_ << ": $" << stoi(amount) 
                    << " from " << sender << " to " << recipient << " at " << exedate_ << ".\n";
                }
                transact temp = {timestamp_, ip_, sender, recipient, stoi(amount), exedate_, os_, tradeCount};
                todo.push(temp);
                tradeCount++;
            }
        } else {
            while (!todo.empty()) {
                execute();
            }
            cin >> field;
            break;
        }
    }
}

void Bank::execute() {
    auto temp1 = users.find(todo.top().sender);
    auto temp2 = users.find(todo.top().recipient);

    int fee = 0;
    int fee1 = 0;
    int fee2 = 0;
    if (todo.top().amount / 100 < 10) {
        fee = 10;
    } else if (todo.top().amount / 100 > 450) {
        fee = 450;
    } else {
        fee = todo.top().amount / 100;
    }

    if (temp1->second.regTime + 50000000000 < todo.top().execTime) {
        fee = (fee * 3) / 4;
    }

    if (todo.top().os) { // s
        fee2 = fee / 2;
        fee1 = (fee - fee / 2);
    } else {
        fee1 = fee;
    }

    // cout << fee1 << ' ' << fee2 << ' ';

    if (temp1->second.balance < todo.top().amount + fee1 || (todo.top().os && temp2->second.balance < fee2)) {
        if (verbose) {
            cout << "Insufficient funds to process transaction " << todo.top().id << ".\n";
        }
        todo.pop();
        return;
    }
    transact temp = todo.top();
    temp.fee = fee1 + fee2;
    todo.pop();

    temp1->second.balance -= temp.amount;
    temp2->second.balance += temp.amount;
    temp1->second.outgoing.push_back(temp);
    temp2->second.incoming.push_back(temp);
    executed.push_back(temp);
    temp1->second.balance -= fee1;
    temp2->second.balance -= fee2;
    if (verbose) {
        cout << "Transaction " << temp.id << " executed at " << temp.execTime << ": $" << temp.amount << " from " 
        << temp.sender << " to " << temp.recipient << ".\n";
    }

    return;
}

void Bank::query() {
    char firstChar;
    while (cin >> firstChar) {
        if (cin.fail()) {
            cerr << "Error: Reading from cin has failed" << endl;
            exit(1);
        }
        if (firstChar == 'l') {
            string begin, end;
            cin >> begin >> end;
            int64_t begin_ = readTime(begin);
            int64_t end_ = readTime(end);
            int num = 0;

            if (begin_ == end_) {
                cout << "List Transactions requires a non-empty time interval.\n";
                continue;
            }
            for (const auto& transaction : executed) {
                if (transaction.execTime > end_) {
                    break;
                }
                if (transaction.execTime >= begin_ ) {
                    if (transaction.amount == 1) {
                        cout << transaction.id << ": " << transaction.sender << " sent " << transaction.amount << " dollar to " 
                        << transaction.recipient << " at " << transaction.execTime << ".\n";
                    } else {
                        cout << transaction.id << ": " << transaction.sender << " sent " << transaction.amount << " dollars to " 
                        << transaction.recipient << " at " << transaction.execTime << ".\n";
                    }
                    num++;
                }
            }
            if (num == 1) {
                cout << "There was " << num << " transaction that was placed between time " << begin_ << " to " << end_ << ".\n";
            } else {
                cout << "There were " << num << " transactions that were placed between time " << begin_ << " to " << end_ << ".\n";
            }
        } else if (firstChar == 'r') {
            string begin, end;
            cin >> begin >> end;
            int64_t begin_ = readTime(begin);
            int64_t end_ = readTime(end);
            int num = 0;

            if (begin_ == end_) {
                cout << "Bank Revenue requires a non-empty time interval.\n";
                continue;
            }

            for (const auto& transaction : executed) {
                if (transaction.execTime > end_) {
                    break;
                }
                if (transaction.execTime >= begin_ ) {
                    // cout << transaction.fee << ' ';
                    num += transaction.fee;
                }
            }

            // cout << "Debug Info: begin_ = " << begin_ << ", end_ = " << end_ << "\n";
            int64_t period = end_ - begin_;
            int64_t year, month, day, hour, minute, second;
            second = period % 100;
            period /= 100;
            minute = period % 100;
            period /= 100;
            hour = period % 100;
            period /= 100;
            day = period % 100;
            period /= 100;
            month = period % 100;
            period /= 100;
            year = period % 100;
            period /= 100;
            
            // cout << "Debug Info: year = " << year << ", month = " << month << ", day = " << day << ", hour = " << hour << ", minute = " << minute << ", second = " << second << "\n";

            cout << "281Bank has collected "<< num << " dollars in fees over";
            if (year > 0) cout << " " << year << " year" << (year == 1 ? "" : "s"); 
            if (month > 0) cout << " " << month << " month" << (month == 1 ? "" : "s");
            if (day > 0) cout << " " << day << " day" << (day == 1 ? "" : "s");
            if (hour > 0) cout << " " << hour << " hour" << (hour == 1 ? "" : "s");
            if (minute > 0) cout << " " << minute << " minute" << (minute == 1 ? "" : "s");
            if (second > 0) cout << " " << second << " second" << (second == 1 ? "" : "s");
            cout << ".\n";
        }else if (firstChar == 'h') {
            string id;
            cin >> id;
            auto person = users.find(id);
            if (person == users.end()) {
                cout << "User " << id << " does not exist.\n";
                continue;
            }
            const auto& user = person->second;
            cout << "Customer " << id << " account summary:\n";
            cout << "Balance: $" << user.balance << "\n";
            cout << "Total # of transactions: " << (user.incoming.size() + user.outgoing.size()) << "\n";

            cout << "Incoming " << user.incoming.size() << ":\n";
            for (size_t i = max(0, (int)(user.incoming.size()) - 10); i < user.incoming.size(); ++i) {
                cout << user.incoming[i].id << ": " << user.incoming[i].sender << " sent " << user.incoming[i].amount << " dollar" << 
                (user.incoming[i].amount == 1 ? "" : "s") << " to " << user.incoming[i].recipient << " at " << user.incoming[i].execTime << ".\n";
            }

            cout << "Outgoing " << user.outgoing.size() << ":\n";
            for (size_t i = max(0, (int)(user.outgoing.size()) - 10); i < user.outgoing.size(); ++i) {
                cout << user.outgoing[i].id << ": " << user.outgoing[i].sender << " sent " << user.outgoing[i].amount << " dollar" << 
                (user.outgoing[i].amount == 1 ? "" : "s") << " to " << user.outgoing[i].recipient << " at " << user.outgoing[i].execTime << ".\n";
            }
        } else {
            string day;
            cin >> day;
            int64_t day_ = readTime(day);
            int64_t begin_ = day_ - (day_ % 1000000);
            int64_t end_ = begin_ + 1000000;
            int count = 0;
            int fee = 0;

            cout << "Summary of [" << begin_ << ", " << end_ << "):\n";
            for (const auto& transaction : executed) {
                if (transaction.execTime > end_) {
                    break;
                }
                if (transaction.execTime >= begin_ ) {
                    cout << transaction.id << ": " << transaction.sender << " sent " << transaction.amount << " dollar" << 
                    (transaction.amount == 1 ? "" : "s") << " to " << transaction.recipient << " at " << transaction.execTime << ".\n";
                    count++;
                    fee += transaction.fee;
                }
            }
            if (count == 1) {
                cout << "There was a total of 1 transaction, 281Bank has collected " << fee << " dollars in fees.\n";
            } else {
                cout << "There were a total of " << count << " transactions, 281Bank has collected " << fee << " dollars in fees.\n";
            }
        } 
    }
}

int64_t Bank::readIP(string a) {
    istringstream iss(a);
    string seg;
    int64_t pin = 0;
    for (int i = 0; i < 4; i++) {
        std::getline(iss, seg, '.');
        // cout << "pin: " << pin << " seg: " << seg << '\n';
        pin = pin * (16 * 16) + stoi(seg);
    }
    return pin;
}

int64_t Bank::readTime(string a) {
    std::istringstream iss(a);
    string seg;
    int64_t result = 0;
    for (int i = 0; i < 6; i++) {
        std::getline(iss, seg, ':');
        result = result * 100 + stoi(seg);
    }
    return result;
}

void print_help() {
    std::cout << "Usage: bank [options]\n"
              << "Options:\n"
              << "  -h, --help       Show this help message and exit\n"
              << "  -f, --file       Specify the registration file\n"
              << "  -v, --verbose    Enable verbose logging\n";
}

void parse_arguments(int argc, char* argv[], std::string& filename, bool& verbose) {
    opterr = static_cast<int>(false);  // Let us handle all error output for command line options
    int choice;
    int index = 0;

    option longOptions[] = {
        {"help", no_argument, nullptr, 'h'},
        {"file", required_argument, nullptr, 'f'},
        {"verbose", no_argument, nullptr, 'v'},
        {nullptr, no_argument, nullptr, 0}
    };

    while ((choice = getopt_long(argc, argv, "hf:v", static_cast<option *>(longOptions), &index)) != -1) {
        switch (choice) {
            case 'h':
                print_help();
                exit(1);    
            case 'f':
                filename = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            default:
                print_help();
                exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::string filename;
    Bank a = Bank();
    parse_arguments(argc, argv, filename, a.verbose);
    // cout << "parsed\n";
    a.registration(filename);
    // cout << "registered\n";
    a.commands();
    // cout << "commanded\n";
    a.query();
    return 0;
}