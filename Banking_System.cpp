// Banking System
//  Purpose : Manage customers, accounts, deposits, withdrawals,
//            transfers and transaction history

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <limits>
#include <algorithm>

using namespace std;

//  Utility
string currentTimestamp() {
    time_t now = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

void printLine() {
    cout << "  ------------------------------------------\n";
}

void printDoubleLine() {
    cout << "  ==========================================\n";
}

int getPositiveInt(const string& prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val && val > 0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  [!] Invalid input. Enter a positive integer.\n";
    }
}

double getPositiveDouble(const string& prompt) {
    double val;
    while (true) {
        cout << prompt;
        if (cin >> val && val > 0.0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  [!] Invalid amount. Enter a value greater than 0.\n";
    }
}

string getLine(const string& prompt) {
    string val;
    cout << prompt;
    getline(cin, val);
    val.erase(0, val.find_first_not_of(" \t"));
    if (!val.empty())
        val.erase(val.find_last_not_of(" \t") + 1);
    return val;
}

//  Transaction
struct Transaction {
    string type;      // DEPOSIT / WITHDRAW / TRANSFER_OUT / TRANSFER_IN
    double amount;
    double balanceAfter;
    string note;
    string timestamp;

    string serialize() const {
        ostringstream oss;
        oss << type << "|" << fixed << setprecision(2) << amount
            << "|" << balanceAfter << "|" << note << "|" << timestamp;
        return oss.str();
    }

    static Transaction deserialize(const string& line) {
        Transaction t;
        istringstream ss(line);
        string amt, bal;
        getline(ss, t.type,      '|');
        getline(ss, amt,         '|');
        getline(ss, bal,         '|');
        getline(ss, t.note,      '|');
        getline(ss, t.timestamp, '|');
        t.amount       = stod(amt);
        t.balanceAfter = stod(bal);
        return t;
    }
};

//  Account
class Account {
public:
    string accountNumber;
    string ownerID;       // customer ID
    string accountType;   // Savings / Current
    double balance;
    vector<Transaction> transactions;

    Account() : balance(0.0) {}

    Account(const string& accNum, const string& ownID,
            const string& type, double initialDeposit)
        : accountNumber(accNum), ownerID(ownID),
          accountType(type), balance(initialDeposit)
    {
        Transaction t;
        t.type         = "DEPOSIT";
        t.amount       = initialDeposit;
        t.balanceAfter = balance;
        t.note         = "Initial deposit";
        t.timestamp    = currentTimestamp();
        transactions.push_back(t);
    }

    bool deposit(double amount, const string& note = "Deposit") {
        if (amount <= 0) return false;
        balance += amount;
        Transaction t;
        t.type         = "DEPOSIT";
        t.amount       = amount;
        t.balanceAfter = balance;
        t.note         = note;
        t.timestamp    = currentTimestamp();
        transactions.push_back(t);
        return true;
    }

    bool withdraw(double amount, const string& note = "Withdrawal") {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        Transaction t;
        t.type         = "WITHDRAW";
        t.amount       = amount;
        t.balanceAfter = balance;
        t.note         = note;
        t.timestamp    = currentTimestamp();
        transactions.push_back(t);
        return true;
    }

    void addTransferOut(double amount, const string& toAcc) {
        balance -= amount;
        Transaction t;
        t.type         = "TRANSFER_OUT";
        t.amount       = amount;
        t.balanceAfter = balance;
        t.note         = "Transfer to " + toAcc;
        t.timestamp    = currentTimestamp();
        transactions.push_back(t);
    }

    void addTransferIn(double amount, const string& fromAcc) {
        balance += amount;
        Transaction t;
        t.type         = "TRANSFER_IN";
        t.amount       = amount;
        t.balanceAfter = balance;
        t.note         = "Transfer from " + fromAcc;
        t.timestamp    = currentTimestamp();
        transactions.push_back(t);
    }

    void printStatement(int limit = 10) const {
        printDoubleLine();
        cout << "  Account   : " << accountNumber << "  [" << accountType << "]\n";
        cout << "  Balance   : PKR " << fixed << setprecision(2) << balance << "\n";
        printLine();
        cout << left << setw(16) << "  Type"
             << setw(12) << "Amount"
             << setw(14) << "Balance"
             << "Note / Timestamp\n";
        printLine();

        int start = max(0, (int)transactions.size() - limit);
        for (int i = start; i < (int)transactions.size(); i++) {
            const Transaction& t = transactions[i];
            cout << "  " << left << setw(14) << t.type
                 << setw(12) << fixed << setprecision(2) << t.amount
                 << setw(14) << t.balanceAfter
                 << t.note << "  " << t.timestamp << "\n";
        }
        printDoubleLine();
    }

    // Serialize for file storage
    string serialize() const {
        ostringstream oss;
        oss << accountNumber << "|" << ownerID << "|"
            << accountType   << "|" << fixed << setprecision(2) << balance
            << "|" << transactions.size() << "\n";
        for (const auto& t : transactions)
            oss << "  TXN:" << t.serialize() << "\n";
        return oss.str();
    }
};

//  Customer
class Customer {
public:
    string customerID;
    string fullName;
    string phone;
    string email;
    string address;
    string createdAt;
    vector<string> accountNumbers; // linked account numbers

    Customer() {}

    Customer(const string& id, const string& name,
             const string& ph, const string& em, const string& addr)
        : customerID(id), fullName(name), phone(ph),
          email(em), address(addr)
    {
        createdAt = currentTimestamp();
    }

    void printInfo() const {
        printDoubleLine();
        cout << "  Customer ID : " << customerID  << "\n";
        cout << "  Name        : " << fullName    << "\n";
        cout << "  Phone       : " << phone       << "\n";
        cout << "  Email       : " << email       << "\n";
        cout << "  Address     : " << address     << "\n";
        cout << "  Joined      : " << createdAt   << "\n";
        cout << "  Accounts    : ";
        if (accountNumbers.empty()) cout << "None";
        else for (const auto& a : accountNumbers) cout << a << "  ";
        cout << "\n";
        printDoubleLine();
    }

    string serialize() const {
        ostringstream oss;
        oss << customerID << "|" << fullName << "|" << phone << "|"
            << email << "|" << address << "|" << createdAt << "|";
        for (int i = 0; i < (int)accountNumbers.size(); i++) {
            if (i > 0) oss << ",";
            oss << accountNumbers[i];
        }
        oss << "\n";
        return oss.str();
    }
};

//  Bank (top-level manager)
class Bank {
private:
    vector<Customer> customers;
    vector<Account>  accounts;
    int nextCustID;
    int nextAccID;

    const string CUST_FILE = "customers.dat";
    const string ACC_FILE  = "accounts.dat";

    // ── ID generators ──────────────────────────────────────
    string newCustomerID() {
        ostringstream oss;
        oss << "C" << setw(4) << setfill('0') << nextCustID++;
        return oss.str();
    }

    string newAccountNumber() {
        ostringstream oss;
        oss << "ACC" << setw(5) << setfill('0') << nextAccID++;
        return oss.str();
    }

    // ── Lookup helpers ─────────────────────────────────────
    Customer* findCustomer(const string& id) {
        for (auto& c : customers)
            if (c.customerID == id) return &c;
        return nullptr;
    }

    Account* findAccount(const string& accNum) {
        for (auto& a : accounts)
            if (a.accountNumber == accNum) return &a;
        return nullptr;
    }

public:
    Bank() : nextCustID(1001), nextAccID(1) {
        loadData();
    }

    // ── Persistence ────────────────────────────────────────
    void saveData() {
        // Save customers
        ofstream cf(CUST_FILE);
        for (const auto& c : customers)
            cf << c.serialize();

        // Save accounts + transactions
        ofstream af(ACC_FILE);
        for (const auto& a : accounts)
            af << a.serialize();
    }

    void loadData() {
        // Load customers
        ifstream cf(CUST_FILE);
        if (cf.is_open()) {
            string line;
            while (getline(cf, line)) {
                if (line.empty()) continue;
                istringstream ss(line);
                Customer c;
                string accs;
                getline(ss, c.customerID, '|');
                getline(ss, c.fullName,   '|');
                getline(ss, c.phone,      '|');
                getline(ss, c.email,      '|');
                getline(ss, c.address,    '|');
                getline(ss, c.createdAt,  '|');
                getline(ss, accs,         '|');
                if (!accs.empty()) {
                    istringstream as(accs);
                    string acc;
                    while (getline(as, acc, ','))
                        if (!acc.empty()) c.accountNumbers.push_back(acc);
                }
                customers.push_back(c);
                // Track highest ID
                int num = stoi(c.customerID.substr(1));
                if (num >= nextCustID) nextCustID = num + 1;
            }
        }

        // Load accounts + transactions
        ifstream af(ACC_FILE);
        if (af.is_open()) {
            string line;
            Account current;
            bool inAccount = false;
            while (getline(af, line)) {
                if (line.empty()) continue;
                if (line.substr(0, 2) != "  ") {
                    if (inAccount) accounts.push_back(current);
                    // Parse account header
                    istringstream ss(line);
                    string bal, txnCount;
                    current = Account();
                    getline(ss, current.accountNumber, '|');
                    getline(ss, current.ownerID,       '|');
                    getline(ss, current.accountType,   '|');
                    getline(ss, bal,                   '|');
                    getline(ss, txnCount,              '|');
                    current.balance = stod(bal);
                    inAccount = true;
                } else {
                    // Transaction line
                    string txnLine = line.substr(6); // remove "  TXN:"
                    current.transactions.push_back(Transaction::deserialize(txnLine));
                }
            }
            if (inAccount) accounts.push_back(current);

            // Track highest account ID
            for (const auto& a : accounts) {
                int num = stoi(a.accountNumber.substr(3));
                if (num >= nextAccID) nextAccID = num + 1;
            }
        }
    }

    // ============================================================
    //  1. Create Customer
    // ============================================================
    void createCustomer() {
        cout << "\n";
        printDoubleLine();
        cout << "  NEW CUSTOMER REGISTRATION\n";
        printDoubleLine();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string name    = getLine("  Full Name   : ");
        string phone   = getLine("  Phone       : ");
        string email   = getLine("  Email       : ");
        string address = getLine("  Address     : ");

        if (name.empty()) {
            cout << "  [!] Name cannot be empty.\n";
            return;
        }

        string id = newCustomerID();
        customers.emplace_back(id, name, phone, email, address);
        saveData();

        cout << "\n  [OK] Customer created successfully!\n";
        cout << "  Customer ID : " << id << "\n";
        printLine();
    }

    // ============================================================
    //  2. Open Account
    // ============================================================
    void openAccount() {
        cout << "\n";
        printDoubleLine();
        cout << "  OPEN NEW ACCOUNT\n";
        printDoubleLine();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string custID = getLine("  Customer ID : ");
        Customer* cust = findCustomer(custID);
        if (!cust) {
            cout << "  [!] Customer not found.\n";
            return;
        }

        cout << "  Account Type:\n";
        cout << "    [1] Savings\n";
        cout << "    [2] Current\n";
        cout << "  Choice : ";
        int typeChoice;
        cin >> typeChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string accType = (typeChoice == 2) ? "Current" : "Savings";

        double initialDeposit = getPositiveDouble("  Initial Deposit (PKR) : ");

        string accNum = newAccountNumber();
        accounts.emplace_back(accNum, custID, accType, initialDeposit);
        cust->accountNumbers.push_back(accNum);
        saveData();

        cout << "\n  [OK] Account opened successfully!\n";
        cout << "  Account No  : " << accNum   << "\n";
        cout << "  Type        : " << accType  << "\n";
        cout << "  Balance     : PKR " << fixed << setprecision(2) << initialDeposit << "\n";
        printLine();
    }

    // ============================================================
    //  3. Deposit
    // ============================================================
    void deposit() {
        cout << "\n";
        printDoubleLine();
        cout << "  DEPOSIT\n";
        printDoubleLine();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string accNum = getLine("  Account No  : ");
        Account* acc  = findAccount(accNum);
        if (!acc) { cout << "  [!] Account not found.\n"; return; }

        double amount = getPositiveDouble("  Amount (PKR): ");
        acc->deposit(amount);
        saveData();

        cout << "\n  [OK] Deposit successful!\n";
        cout << "  New Balance : PKR " << fixed << setprecision(2) << acc->balance << "\n";
        printLine();
    }

    // ============================================================
    //  4. Withdraw
    // ============================================================
    void withdraw() {
        cout << "\n";
        printDoubleLine();
        cout << "  WITHDRAWAL\n";
        printDoubleLine();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string accNum = getLine("  Account No  : ");
        Account* acc  = findAccount(accNum);
        if (!acc) { cout << "  [!] Account not found.\n"; return; }

        cout << "  Current Balance : PKR " << fixed << setprecision(2) << acc->balance << "\n";
        double amount = getPositiveDouble("  Amount (PKR)   : ");

        if (!acc->withdraw(amount)) {
            cout << "  [!] Insufficient balance.\n";
            return;
        }
        saveData();

        cout << "\n  [OK] Withdrawal successful!\n";
        cout << "  New Balance : PKR " << fixed << setprecision(2) << acc->balance << "\n";
        printLine();
    }

    // ============================================================
    //  5. Transfer
    // ============================================================
    void transfer() {
        cout << "\n";
        printDoubleLine();
        cout << "  FUND TRANSFER\n";
        printDoubleLine();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string fromNum = getLine("  From Account : ");
        string toNum   = getLine("  To Account   : ");

        if (fromNum == toNum) {
            cout << "  [!] Cannot transfer to the same account.\n";
            return;
        }

        Account* from = findAccount(fromNum);
        Account* to   = findAccount(toNum);

        if (!from) { cout << "  [!] Source account not found.\n";      return; }
        if (!to)   { cout << "  [!] Destination account not found.\n"; return; }

        cout << "  Your Balance : PKR " << fixed << setprecision(2) << from->balance << "\n";
        double amount = getPositiveDouble("  Amount (PKR) : ");

        if (amount > from->balance) {
            cout << "  [!] Insufficient balance.\n";
            return;
        }

        from->addTransferOut(amount, toNum);
        to->addTransferIn(amount, fromNum);
        saveData();

        cout << "\n  [OK] Transfer successful!\n";
        cout << "  From Balance : PKR " << fixed << setprecision(2) << from->balance << "\n";
        cout << "  To Balance   : PKR " << fixed << setprecision(2) << to->balance   << "\n";
        printLine();
    }

    // ============================================================
    //  6. View Account Statement
    // ============================================================
    void viewStatement() {
        cout << "\n";
        printDoubleLine();
        cout << "  ACCOUNT STATEMENT\n";
        printDoubleLine();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string accNum = getLine("  Account No  : ");
        Account* acc  = findAccount(accNum);
        if (!acc) { cout << "  [!] Account not found.\n"; return; }

        acc->printStatement(15);
    }

    // ============================================================
    //  7. View Customer Info
    // ============================================================
    void viewCustomer() {
        cout << "\n";
        printDoubleLine();
        cout << "  CUSTOMER PROFILE\n";
        printDoubleLine();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string custID = getLine("  Customer ID : ");
        Customer* cust = findCustomer(custID);
        if (!cust) { cout << "  [!] Customer not found.\n"; return; }

        cust->printInfo();

        // Also show balance summary for each account
        if (!cust->accountNumbers.empty()) {
            cout << "  Account Summary:\n";
            printLine();
            for (const auto& aNum : cust->accountNumbers) {
                Account* acc = findAccount(aNum);
                if (acc)
                    cout << "  " << left << setw(12) << acc->accountNumber
                         << setw(10) << acc->accountType
                         << "PKR " << fixed << setprecision(2) << acc->balance << "\n";
            }
            printLine();
        }
    }

    // ============================================================
    //  8. List All Customers
    // ============================================================
    void listCustomers() {
        cout << "\n";
        printDoubleLine();
        cout << "  ALL CUSTOMERS (" << customers.size() << " total)\n";
        printDoubleLine();

        if (customers.empty()) {
            cout << "  No customers registered yet.\n";
        } else {
            cout << left << setw(10) << "  ID"
                 << setw(22) << "Name"
                 << setw(14) << "Phone"
                 << "Accounts\n";
            printLine();
            for (const auto& c : customers) {
                cout << "  " << left << setw(8)  << c.customerID
                     << setw(22) << c.fullName
                     << setw(14) << c.phone;
                for (const auto& a : c.accountNumbers) cout << a << " ";
                cout << "\n";
            }
        }
        printDoubleLine();
    }

    // ============================================================
    //  Main Menu
    // ============================================================
    void run() {
        cout << "\n";
        cout << "  +==========================================+\n";
        cout << "  |     BANKING SYSTEM  --  CodeAlpha       |\n";
        cout << "  +==========================================+\n\n";

        int choice;
        while (true) {
            cout << "  +------------------------------------------+\n";
            cout << "  |              MAIN MENU                   |\n";
            cout << "  +------------------------------------------+\n";
            cout << "  |  [1]  New Customer                       |\n";
            cout << "  |  [2]  Open Account                       |\n";
            cout << "  |  [3]  Deposit                            |\n";
            cout << "  |  [4]  Withdraw                           |\n";
            cout << "  |  [5]  Fund Transfer                      |\n";
            cout << "  |  [6]  Account Statement                  |\n";
            cout << "  |  [7]  Customer Profile                   |\n";
            cout << "  |  [8]  List All Customers                 |\n";
            cout << "  |  [0]  Exit                               |\n";
            cout << "  +------------------------------------------+\n";
            cout << "  Choice : ";

            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "  [!] Invalid input.\n\n";
                continue;
            }

            switch (choice) {
                case 1: createCustomer(); break;
                case 2: openAccount();    break;
                case 3: deposit();        break;
                case 4: withdraw();       break;
                case 5: transfer();       break;
                case 6: viewStatement();  break;
                case 7: viewCustomer();   break;
                case 8: listCustomers();  break;
                case 0:
                    cout << "\n  Thank you for using CodeAlpha Banking. Goodbye!\n\n";
                    return;
                default:
                    cout << "  [!] Invalid option. Try again.\n";
            }
            cout << "\n";
        }
    }
};

// ============================================================
//  Main
// ============================================================
int main() {
    Bank bank;
    bank.run();
    return 0;
}
