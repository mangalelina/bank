#ifndef bank_hpp
#define bank_hpp
#include <stdio.h>
#include <string>
#include <iostream>
#include <queue>
#include <getopt.h>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

using namespace std;

/**
 User Class:
    Contains data about each user
    Getters, Setters,
    Functions to find, add or remove IP addresses, and
    Trackers for incoming and outgoing transactions
 */
class User {
private:
    uint64_t registrationTime;
    int pin;
    uint32_t balance;
    unordered_set<string> validIP;
    vector<size_t> incomingTransactions;
    vector<size_t> outgoingTransactions;

public:
    
    uint64_t convertStringToTimestamp(string timestamp) {
        size_t size = timestamp.size();
        uint64_t time = uint64_t(timestamp[0] - 48);
        for (size_t i = 1; i < size; i++) {
            if (timestamp[i] == ':') {}
            else {
                time *= 10ULL;
                time += uint64_t(timestamp[i] - 48);
            }
        }
        return time;
    }
    
    User(string reg_, string pin_, string balance_) {
        registrationTime = convertStringToTimestamp(reg_);
        pin = stoi(pin_);
        balance = (uint32_t)stoi(balance_);
    }
    
    //getters
    uint64_t getRegTime() { return registrationTime;}
    int getPin() { return pin;}
    uint32_t getBalance() {return balance; }
    
    // setter
    void updateBalance(uint32_t balance_) {
        balance = balance_;
    }
    
    // IP Address functions
    void addIP(string IP) {validIP.insert(IP);}
    void removeIP(string IP) {validIP.erase(IP);}
    bool isActive() {
        if (validIP.size() > 0) return true;
        else return false;
    }
    bool matchIP(string ip) {
        if (validIP.find(ip) != validIP.end()) return true;
        else return false;
    }
    
    // manage users' transaction history
    void addIncomingTransaction(size_t transactionID) {
        incomingTransactions.push_back(transactionID);
        //cerr << "size of incoming for user " << userID << " is " << incomingTransactions.size() << "\n";
    }
    void addOutgoingTransaction(size_t transactionID) {
        outgoingTransactions.push_back(transactionID);
        //cerr << "size of outgoing for user " << userID << " is " << outgoingTransactions.size() << "\n";
    }
    size_t getNumOfTransactions() {return incomingTransactions.size() + outgoingTransactions.size();}
    size_t getNumOfIncomingTransactions() {return incomingTransactions.size();}
    size_t getNumOfOutgoingTransactions() {return outgoingTransactions.size();}
    
    vector<size_t> getIncomingIndicesForHistory() {
        if (incomingTransactions.size() > 10) {
            auto first = incomingTransactions.end() - 10;
            auto last = incomingTransactions.end();
            return vector<size_t>(first, last);
        }
        else {
            return incomingTransactions;
        }
    }
    
    vector<size_t> getOutgoingIndicesForHistory() {
        if (outgoingTransactions.size() > 10) {
            auto first = outgoingTransactions.end() - 10;
            auto last = outgoingTransactions.end();
            return vector<size_t>(first, last);
        }
        else {
            return outgoingTransactions;
        }
    }
};


/**
 Transaction  Class:
    Contains data about each transaction
    Getters, Setters
 */

class Transaction {
private:
    string senderIP;
    string senderID;
    string recipientID;
    uint32_t amount;
    uint64_t execTime;
    char feesPolicy;
    size_t transactionID;
    uint32_t senderFees = 0;
    uint32_t recipientFees = 0;
    uint32_t fees = 0;
    
    
public:
    
    Transaction(string senderIP_, string senderID_, string recipientID_, uint32_t amount_, uint64_t execTime_, char feesPolicy_, size_t transactionID_, uint64_t senderRegTime_) : senderIP(senderIP_), senderID(senderID_), recipientID(recipientID_), amount(amount_), execTime(execTime_), feesPolicy(feesPolicy_), transactionID(transactionID_) {
        
        
        fees = amount / 100;
        //cerr << "fees basic = " << fees << "\n";
        
        if (fees < 10) fees = 10;
        if (fees > 450) fees = 450;
        //cerr << "fees after min/max = " << fees << "\n";
        
        //apply loyalty discounts
        //cerr << "exec time = " << execTime << ", senderRegtime = " << senderRegTime_ << ", diff = " << execTime - senderRegTime_ << "\n";
        if (execTime - senderRegTime_ > 50000000000ULL)
            fees = (fees * 3) / 4;
        
        //cerr << "fees after loyalty = " << fees << "\n";
        
        // apply feesPolicy
        if (feesPolicy == 'o') senderFees = fees;
        else senderFees = (fees+1)/2;
        recipientFees = fees-senderFees;
        //cerr << "senderFees = " << senderFees << "\n";
        //cerr << "recipientFees = " << recipientFees << "\n";
        
    }
    
    // getters
    string getSenderID() const { return senderID; }
    string getRecipientID() const { return recipientID; }
    uint32_t getAmount() const { return amount; }
    uint64_t getExecTime() const { return execTime;}
    char getFeesPolicy() const { return feesPolicy; }
    size_t getTransactionID() const { return transactionID;}
    uint32_t getSenderFees() const { return senderFees;}
    uint32_t getRecipientFees() const { return recipientFees;}
    uint32_t getFees() const { return fees;}
};

/**
This is a functor used to compute pending transactions PQ
 */
class TransactionCompare {
public:
    bool operator()(Transaction& a, Transaction& b){
        if (a.getExecTime() < b.getExecTime()) {
            //cerr << "a's transaction time = " << a.getExecTime() << " is less than b's transaction time = " << b.getExecTime() << "\n";
            return false;
        }
        if (a.getExecTime() > b.getExecTime()) {
            //cerr << "a's transaction time = " << a.getExecTime() << " is greater than b's transaction time = " << b.getExecTime() << "\n";
            return true;
        }
        else {
            //cerr << "a's transaction time = " << a.getExecTime() << " is equal to b's transaction time = " << b.getExecTime() << "\n";

            if (a.getTransactionID() < b.getTransactionID()) {
                //cerr << "a's transactionID = " << a.getTransactionID() << " is less than b's transactionID = " << b.getTransactionID() << "\n";
                return false;
            }
            else {
                //cerr << "a's transactionID = " << a.getTransactionID() << " is greater than b's transactionID = " << b.getTransactionID() << "\n";
                return true;
            }
        }
    }
};

/**
Bank Class:
    contains info about input modes
    reads registration data and holds all users in an unordered_map
    implements instructions and queries
    executes transactions
 */
class Bank {
private:
    bool verbose;
    string filename;
    int gotopt;
    
    // pq with all pending transactions
    priority_queue<Transaction, vector<Transaction>, TransactionCompare> pending_transactions;
    
    // uses user_id as key to the object USER containing all relevant data
    unordered_map<string, User> banker;
    
    // uses transaction_id as index
    vector<Transaction> transactor;
    size_t transactionID = 0;
    
    // each item represents a transactionID in the order on execTime
    vector<size_t> executedSorted;
    
    uint64_t prev_place_timestamp = 0ULL;
    
    
public:
    Bank(int argc, char * argv[]) {
        verbose = false;
        option longOpts[] = {
            { "file", required_argument, nullptr, 'f' },
            { "help", no_argument, nullptr, 'h' },
            { "verbose", no_argument, nullptr, 'v' },
            { nullptr, 0, nullptr, '\0' }
        };
        
        while((gotopt = getopt_long(argc, argv, "f:hv" , longOpts, nullptr)) != -1) {
            switch (gotopt) {
                case 'f': {
                    filename = (optarg);
                    break;
                }
                case 'v': {
                    verbose = true;
                    break;
                }
                case 'h': {
                    cout << "Read the spec\n";
                    exit(0);
                }
            }
        }
        registeration();
    }
    
    uint64_t convertStringToTimestamp(string timestamp) {
        size_t size = timestamp.size();
        uint64_t time = uint64_t(timestamp[0] - 48);
        for (size_t i = 1; i < size; i++) {
            if (timestamp[i] == ':') {}
            else {
                time *= 10ULL;
                time += uint64_t(timestamp[i] - 48);
            }
        }
        return time;
    }
    
    void registeration() {
        ifstream regFile;
        regFile.open(filename);
        
        if (!regFile.is_open()) cout << "Registration file failed to open.\n";
        else {
            string timestamp;
            string user_id;
            string pin_s;
            string starting_balance;
            while(getline(regFile, timestamp, '|')) {
                getline(regFile, user_id, '|');
                getline(regFile, pin_s, '|');
                getline(regFile, starting_balance, '\n');
                User u(timestamp, pin_s, starting_balance);
                pair<std::string,User> newUser (user_id,u);
                banker.insert(newUser);
            }
            instructions();
        }
    }
    
    void instructions() {
        
        string instruction;
        cin >> instruction;
        while (instruction != "$$$") {
            if (instruction[0] == '#') {
                string junk;
                getline(cin, junk);
            }
            else if (instruction[0] == 'l') {
                string id, ip;
                int pin;
                cin >> id >> pin >> ip;
                login(id, pin, ip);
            }
            else if (instruction[0] == 'o') {
                string id, ip;
                cin >> id >> ip;
                logout(id, ip);
            }
            else if (instruction[0] == 'p') {
                string timestamp, ip, senderID, recipientID, execDate;
                uint32_t amount;
                char feesPolicy;
                cin >> timestamp >> ip >> senderID >> recipientID >> amount >> execDate >> feesPolicy;
                place(timestamp, ip, senderID, recipientID, amount, execDate, feesPolicy);
            }
            cin >> instruction;
        }
        while (!pending_transactions.empty()) {
            executeTransaction(pending_transactions.top());
            pending_transactions.pop();
        }
        
        char query;
        while (cin >> query) {
            if (query == 'l') {
                string startTime, endTime;
                cin >> startTime >> endTime;
                list(startTime, endTime);
            }
            else if (query == 'r') {
                string startTime, endTime;
                cin >> startTime >> endTime;
                revenue(startTime, endTime);
            }
            else if (query == 'h') {
                string user;
                cin >> user;
                history(user);
            }
            else if (query == 's') {
                string time;
                cin >> time;
                summary(time);
            }
            else {}
        }
    }
    
    // check id and pin, if valid add ip
    void login(string id, int pin, string ip) {
        auto it = banker.find(id);
        if (it == banker.end()) {// ID not found
            if (verbose) {
                cout << "Failed to log in " << id << ".\n";
                //cerr << "Failed to log in " << id << ".\n";
            }
        }
        else {
            if (it->second.getPin() == pin) {
                it->second.addIP(ip);
                if (verbose)  {
                    cout << "User " << id << " logged in.\n";
                    //cerr << "User " << id << " logged in.\n";
                }
            }
            else {// Pin does not match
                if (verbose) {
                    cout << "Failed to log in " << id << ".\n";
                    //cerr << "Failed to log in " << id << ".\n";
                }
            }
        }
        
    }
    
    // check id and if active, if valid, remove ip
    void logout(string id, string ip) {
        auto it = banker.find(id);
        if (it == banker.end()) { // user does not exist
            if (verbose) {
                cout << "Failed to log out " << id << ".\n";
                //cerr << " user doesn't exist Failed to log out " << id << ".\n";
            }
        }
        else {
            if (it->second.isActive()) {
                if (!it->second.matchIP(ip)) { // IP match not found
                    if (verbose) {
                        cout << "Failed to log out " << id << ".\n";
                        //cerr << "ip doesn't match Failed to log out " << id << ".\n";
                    }
                }
                else {//valid
                    it->second.removeIP(ip);
                    if (verbose) {
                        cout << "User " << id << " logged out.\n";
                        //cerr << "User " << id << " logged out.\n";
                    }
                }
            }
            else { // user not active
                if (verbose) {
                    cout << "Failed to log out " << id << ".\n";
                    //cerr << "not active Failed to log out " << id << ".\n";
                }
            }
        }
        
    }
    
    
    // conduct checks at placement, check for fraud, else create transaction object and add to pq
    void place(string timestamp, string sender_ip, string sender_id, string recepient_id, uint32_t amount, string exec_date_s, char fees_policy) {
        
        
        uint64_t exec = convertStringToTimestamp(exec_date_s);
        uint64_t placed = convertStringToTimestamp(timestamp);
        
        
        if (placed < prev_place_timestamp) {
            //cerr << "prev: " << prev_place_timestamp << " and placed: " << placed << "\n";
            cerr << "Invalid decreasing timestamp in 'place' command.\n";
            exit(1);
        }
        if (exec < placed) {
            //cerr << "exec: " << exec << " and placed: " << placed << "\n";
            cerr << "You cannot have an execution date before the current timestamp.\n";
            exit(1);
        }
       
        // conduct checks that occur at placement time
        auto sender = banker.find(sender_id);
        auto recipient = banker.find(recepient_id);
        
        if (exec - placed > 3000000ULL) {
            if (verbose) {
                cout << "Select a time less than three days in the future.\n";
                //cerr << "Select a time less than three days in the future.\n";
            }
        }
        else if (banker.find(sender_id) == banker.end()) {// sender ID not found
            if (verbose) {
                cout << "Sender " << sender_id << " does not exist.\n";
                //cerr << "Sender " << sender_id << " does not exist.\n";
            }
        }
        else if (banker.find(recepient_id) == banker.end()) { // recipient ID not found
            if (verbose) {
                cout << "Recipient " << recepient_id << " does not exist.\n";
                //cerr << "Recipient " << recepient_id << " does not exist.\n";
            }
        }
        else if (exec < sender->second.getRegTime() || exec < recipient->second.getRegTime()) {
            if (verbose) {
                cout << "At the time of execution, sender and/or recipient have not registered.\n";
                //cerr << "At the time of execution, sender and/or recipient have not registered.\n";
            }
        }
        else if (!sender->second.isActive()) {// sender is not in active session
            if (verbose) {
                cout << "Sender " << sender_id << " is not logged in.\n";
                //cerr << "Sender " << sender_id << " is not logged in.\n";
            }
        }
        
        // Check for fradulent activity
        else if (!sender->second.matchIP(sender_ip)) {
            if (verbose) {
                cout << "Fraudulent transaction detected, aborting request.\n";
                //cerr << "Fraudulent transaction detected, aborting request.\n";
            }
        }
        
        
        // valid, create transaction object
        else {
            prev_place_timestamp = placed;
         
                while(!pending_transactions.empty() && pending_transactions.top().getExecTime() <= placed) {
                    executeTransaction(pending_transactions.top());
                    pending_transactions.pop();
                }
            
            transactor.push_back(Transaction(sender_ip, sender_id, recepient_id, amount, exec, fees_policy, transactionID, banker.find(sender_id)->second.getRegTime()));
            
            if (verbose) {
                cout << "Transaction placed at " << placed << ": $" << amount << " from " << sender_id << " to " << recepient_id << " at " << exec << ".\n";
                //cerr << "Transaction placed at " << placed << ": $" << amount << " from " << sender_id << " to " << recepient_id << " at " << exec << ".\n";
            }
            
            // store in pending transactions
            pending_transactions.push(transactor[transactionID]);
            transactionID++;
        }
    }
        
    // checks for sufficient funds, transfers funds, deducts fees, adds to vector of executed transactions, adds to sender's outgoing and recipient's incoming vectors
    void executeTransaction(Transaction transaction) {
        //cerr << "EXECUTING transaction " << transaction.getTransactionID() << "\n";
        
        auto sender = banker.find(transaction.getSenderID());
        auto recipient = banker.find(transaction.getRecipientID());
        
        
        
        //cerr << "sender balance is:  " << sender->second.getBalance() <<  ", sender fees is: " << transaction.getSenderFees() << ", amount is: " << transaction.getAmount() << "\n";
        //cerr << "recipient balance is: " << recipient->second.getBalance() << ", recipient fees is: " << transaction.getRecipientFees() << "\n";
        
        if (sender->second.getBalance() < (transaction.getAmount() + transaction.getSenderFees())) {
            if (verbose) {
                cout << "Insufficient funds to process transaction " << transaction.getTransactionID() << ".\n";
                //cerr << "Insufficient funds to process transaction " << transaction.getTransactionID() << ". sender doesn't have enough\n";
            }
        }
        else if (transaction.getFeesPolicy() == 's' && recipient->second.getBalance() < transaction.getRecipientFees()) {
            if (verbose) {
                cout << "Insufficient funds to process transaction " << transaction.getTransactionID() << ".\n";
                //cerr << "Insufficient funds to process transaction " << transaction.getTransactionID() << ". recipient doesn't have enough\n";
            }
        }
        else {
            // transfer funds
            //cerr << "updating balance to transfer funds, sender\n";
            sender->second.updateBalance(sender->second.getBalance() - transaction.getAmount());
            //cerr << "sender = " << sender->second.getBalance() << "\n";
            //cerr << "updating balance to transfer funds, recipient\n";
            recipient->second.updateBalance(recipient->second.getBalance() + transaction.getAmount());
            //cerr << " rec = " << recipient->second.getBalance() << "\n";
            
            // deduct fees
            //cerr << "updating balance to deduct fees, sender\n";
            sender->second.updateBalance(sender->second.getBalance() - transaction.getSenderFees());
            //cerr << "sender  = " << sender->second.getBalance() << "\n";
            //cerr << "updating balance to deduct fees, recipient\n";
            recipient->second.updateBalance(recipient->second.getBalance() - transaction.getRecipientFees());
           //cerr << " rec = " << recipient->second.getBalance() << "\n";
            //cerr << "After fees deduction, senderBalance = " << sender->second.getBalance() << " and recipientBalance = " << recipient->second.getBalance() << "\n";
                        
            // add to list of executed transactions
            executedSorted.push_back(transaction.getTransactionID());
            
            // add to sender's outgoing transactions
            sender->second.addOutgoingTransaction(transaction.getTransactionID());
            
            // add to recipient's incoming transactions
            recipient->second.addIncomingTransaction(transaction.getTransactionID());
            
            if (verbose) {
                cout << "Transaction executed at " << transaction.getExecTime() << ": $" << transaction.getAmount() << " from " << transaction.getSenderID() << " to " << transaction.getRecipientID() << ".\n";
                //cerr << "Transaction executed at " << transaction.getExecTime() << ": $" << transaction.getAmount() << " from " << transaction.getSenderID() << " to " << transaction.getRecipientID() << ".\n";
            }
            //cerr << "sender balance = " << sender->second.getBalance() << " rec balance: " << recipient->second.getBalance() << "\n";
            //cerr << "---------------\n";
        }
    }
    
    // given a timestamp, find the first spot that time would be in the executedSorted vector
    size_t lowBoundExecSorted(size_t left, size_t right, uint64_t time) {
        while (right > left) {
            size_t mid = left + (right-left) / 2;
            if (transactor[executedSorted[mid]].getExecTime() < time) left = mid + 1;
            else right = mid;
        }
        return left;
    }
    
    // prints all the transaction that occured between given timestamps using executedSorted
    void list(string x, string y) {
        //cerr << "LIST\n";
        uint64_t startTime = convertStringToTimestamp(x);
        uint64_t endTime = convertStringToTimestamp(y);
        if (executedSorted.size() > 0) {
            //cerr << "executedSorted.size() = " << executedSorted.size() << "\n";
            size_t s = 0;
            size_t e = executedSorted.size()-1;
            size_t index = lowBoundExecSorted(s, e, startTime);
            int numTransactions = 0;
            while (index <= e && transactor[executedSorted[index]].getExecTime() < endTime) {
                if (transactor[executedSorted[index]].getAmount() != 1) {
                    cout << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollars to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
                    //cerr << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollars to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
                    numTransactions++;
                    index++;
                }


                else {
                    cout << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollar to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
                    //cerr << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollar to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
                    numTransactions++;
                    index++;
                }
            }
            
            if (numTransactions != 1) {
                cout << "There were " << numTransactions << " transactions that were placed between time " << startTime << " to " << endTime << ".\n";
                //cerr << "There were " << numTransactions << " transactions that were placed between time " << startTime << " to " << endTime << ".\n";
            }
            else {
                cout << "There was 1 transaction that was placed between time " << startTime << " to " << endTime << ".\n";
                //cerr << "There was 1 transaction that was placed between time " << startTime << " to " << endTime << ".\n";
            }
        }
        
        
    }
    
    // loops through all transactions executed in the given time range and adds up the fees they paid
    void revenue(string x, string y) {
        uint64_t startTime = convertStringToTimestamp(x);
        uint64_t endTime = convertStringToTimestamp(y);
        
       if (executedSorted.size() > 0) {
            size_t s = 0;
            size_t e = executedSorted.size()-1;
            size_t index = lowBoundExecSorted(s, e, startTime);
            uint32_t sum = 0;
            
            while (index <= e && transactor[executedSorted[index]].getExecTime() < endTime) {
                
                sum += transactor[executedSorted[index]].getFees();
                index++;
            }
            
            string sum_s = to_string(sum);
            string result = "281Bank has collected ";
            result.append(sum_s);
            result.append(" dollars in fees over");
            
            
            uint64_t startTime = convertStringToTimestamp(x);
            uint64_t endTime = convertStringToTimestamp(y);
            uint64_t time_diff = endTime - startTime;
           //cerr << "time_diff = " << time_diff << "\n";
            string diff = to_string(time_diff);
           //cerr << "diff = " << diff << "\n";
                       
           string yDigits = "0";
           string mthDigits = "0";
           string dDigits = "0";
           string hDigits = "0";
           string mnDigits = "0";
           string sDigits = "0";
           
           if (diff.size() == 12) {
                yDigits = diff.substr(0, 2);
                mthDigits = diff.substr(2, 2);
                dDigits = diff.substr(4, 2);
                hDigits = diff.substr(6, 2);
                mnDigits = diff.substr(8, 2);
                sDigits = diff.substr(10, 2);
           }
           else if (diff.size() == 11) {
                yDigits = diff.substr(0, 1);
                mthDigits = diff.substr(1, 2);
                dDigits = diff.substr(3, 2);
                hDigits = diff.substr(5, 2);
                mnDigits = diff.substr(7, 2);
                sDigits = diff.substr(9, 2);
           }
           else if (diff.size() == 10) {
                mthDigits = diff.substr(0, 2);
                dDigits = diff.substr(2, 2);
                hDigits = diff.substr(4, 2);
                mnDigits = diff.substr(6, 2);
                sDigits = diff.substr(8, 2);
           }
           else if (diff.size() == 9) {
                mthDigits = diff.substr(0, 1);
                dDigits = diff.substr(1, 2);
                hDigits = diff.substr(3, 2);
                mnDigits = diff.substr(5, 2);
                sDigits = diff.substr(7, 2);
           }
           else if (diff.size() == 8) {
                dDigits = diff.substr(0, 2);
                hDigits = diff.substr(2, 2);
                mnDigits = diff.substr(4, 2);
                sDigits = diff.substr(6, 2);
           }
           else if (diff.size() == 7) {
                dDigits = diff.substr(0, 1);
                hDigits = diff.substr(1, 2);
                mnDigits = diff.substr(3, 2);
                sDigits = diff.substr(5, 2);
           }
           else if (diff.size() == 6) {
                hDigits = diff.substr(0, 2);
                mnDigits = diff.substr(2, 2);
                sDigits = diff.substr(4, 2);
           }
           else if (diff.size() == 5) {
                hDigits = diff.substr(0, 1);
                mnDigits = diff.substr(1, 2);
                sDigits = diff.substr(3, 2);
           }
           else if (diff.size() == 4) {
                mnDigits = diff.substr(0, 2);
                sDigits = diff.substr(2, 2);
           }
           else if (diff.size() == 3) {
                mnDigits = diff.substr(0, 1);
                sDigits = diff.substr(1, 2);
           }
           else if (diff.size() == 2) {
                sDigits = diff.substr(0, 2);
           }
           else if (diff.size() == 1) {
                sDigits = diff.substr(0, 1);
           }
           else {
                // no time diff
           }
           
           bool yrs = false;
           bool mths = false;
           bool days = false;
           bool hrs = false;
           bool mins = false;
           bool secs = false;
           
           if (yDigits != "0" && yDigits != "00") {
               yrs = true;
               result.append(" ");
               result.append(to_string(stoi(yDigits)));
               if (stoi(yDigits) == 1) result.append(" year");
               else result.append(" years");
               //cerr << result << "\n";
           }
           if (mthDigits != "0" && mthDigits != "00") {
               mths = true;
               result.append(" ");
               result.append(to_string(stoi(mthDigits)));
               if (stoi(mthDigits) == 1) result.append(" month");
               else result.append(" months");
               //cerr << result << "\n";
           }
           if (dDigits != "0" && dDigits != "00") {
               days = true;
               result.append(" ");
               result.append(to_string(stoi(dDigits)));
               if (stoi(dDigits) == 1) result.append(" day");
               else result.append(" days");
               //cerr << result << "\n";
           }
           if (hDigits != "0" && hDigits != "00") {
               hrs = true;
               result.append(" ");
               result.append(to_string(stoi(hDigits)));
               if (stoi(hDigits) == 1) result.append(" hour");
               else result.append(" hours");
               //cerr << result << "\n";
           }
           if (mnDigits != "0" && mnDigits != "00") {
               mins = true;
               result.append(" ");
               result.append(to_string(stoi(mnDigits)));
               if (stoi(mnDigits) == 1) result.append(" minute");
               else result.append(" minutes");
               //cerr << result << "\n";
           }
           if (sDigits != "0" && sDigits != "00") {
               secs = true;
               result.append(" ");
               result.append(to_string(stoi(sDigits)));
               if (stoi(sDigits) == 1) result.append(" second");
               else result.append(" seconds");
               //cerr << result << "\n";
           }
           
           if (!yrs && !mths && !days && !hrs && !mins && !secs) {
               result.append(" ");
           }
           
 
            result.append(".\n");
            //cerr << result;
            cout << result;
        }
        
    }
    
    void history(string user_id) {
        auto it = banker.find(user_id);
        
        if (it == banker.end()) { // user not found
            cout << "User " << user_id << " does not exist.\n";
            //cerr << "User " << user_id << " does not exist.\n";
            return;
        }
        
        cout << "Customer " << user_id << " account summary:\n";
        cout << "Balance: $" << it->second.getBalance() << "\n";
        cout << "Total # of transactions: " << it->second.getNumOfTransactions() << "\n";
        
        //cerr << "Customer " << user_id << " account summary:\n";
        //cerr << "Balance: $" << it->second.getBalance() << "\n";
        //cerr << "Total # of transactions: " << it->second.getNumOfTransactions() << "\n";
        
        
        cout << "Incoming " << it->second.getNumOfIncomingTransactions() << ":\n";
        //cerr << "Incoming " << it->second.getNumOfIncomingTransactions() << ":\n";
        vector<size_t> incomingTransactions = it->second.getIncomingIndicesForHistory();
        for (auto elt : incomingTransactions) {
            if (transactor[elt].getAmount() != 1) {
                cout << elt << ": " << transactor[elt].getSenderID() << " sent " << transactor[elt].getAmount() << " dollars to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
                //cerr << elt << ": " << transactor[elt].getSenderID() << " sent " << transactor[elt].getAmount() << " dollars to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
            }
            else {
                cout << elt << ": " << transactor[elt].getSenderID() << " sent 1 dollar to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
                //cerr << elt << ": " << transactor[elt].getSenderID() << " sent 1 dollar to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
            }
        }
        
        
        cout << "Outgoing " << it->second.getNumOfOutgoingTransactions() << ":\n";
        //cerr << "Outgoing " << it->second.getNumOfOutgoingTransactions() << ":\n";
        vector<size_t> outgoingTransactions = it->second.getOutgoingIndicesForHistory();
        for (auto elt : outgoingTransactions) {
            if (transactor[elt].getAmount() != 1) {
                cout << elt << ": " << transactor[elt].getSenderID() << " sent " << transactor[elt].getAmount() << " dollars to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
                //cerr << elt << ": " << transactor[elt].getSenderID() << " sent " << transactor[elt].getAmount() << " dollars to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
            }
            else {
                cout << elt << ": " << transactor[elt].getSenderID() << " sent 1 dollar to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
                //cerr << elt << ": " << transactor[elt].getSenderID() << " sent 1 dollar to " << transactor[elt].getRecipientID() << " at " << transactor[elt].getExecTime() << ".\n";
            }
        }
    }
    
    void summary(string timestamp) {
        if (executedSorted.size() == 0) return;
        
        uint64_t startTime = convertStringToTimestamp(timestamp);
        startTime /= 1000000;
        startTime *= 1000000;
        uint64_t endTime = startTime / 1000000;
        endTime++;
        endTime *= 1000000;
        
        cout << "Summary of [" << startTime << ", " << endTime << "):\n";
        //cerr << "Summary of [" << startTime << ", " << endTime << "):\n";
        
        int numTransactions = 0;
        uint32_t sum = 0;
      
        size_t s = 0;
        size_t e = executedSorted.size()-1;
        size_t index = lowBoundExecSorted(s, e, startTime);
        while (index <= e && transactor[executedSorted[index]].getExecTime() < endTime) {
            
            
            sum += transactor[executedSorted[index]].getFees();
            if (transactor[executedSorted[index]].getAmount() != 1) {
                cout << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollars to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
                //cerr << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollars to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
            }
                
            else {
                cout << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollar to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
                //cerr << transactor[executedSorted[index]].getTransactionID() << ": " << transactor[executedSorted[index]].getSenderID() << " sent " << transactor[executedSorted[index]].getAmount() << " dollar to " << transactor[executedSorted[index]].getRecipientID() << " at " << transactor[executedSorted[index]].getExecTime() << ".\n";
            }
            
            numTransactions++;
            index++;
        }
        
        if (numTransactions != 1) {
            cout << "There were a total of " << numTransactions << " transactions, 281Bank has collected " << sum << " dollars in fees.\n";
            //cerr << "There were a total of " << numTransactions << " transactions, 281Bank has collected " << sum << " dollars in fees.\n";
        }
        else {
            cout << "There was a total of " << numTransactions << " transaction, 281Bank has collected " << sum << " dollars in fees.\n";
            //cerr << "There was a total of " << numTransactions << " transaction, 281Bank has collected " << sum << " dollars in fees.\n";
        }
    }
    
    
    
};


#endif /* bank_hpp */
