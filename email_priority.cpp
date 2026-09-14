// email_priority.cpp
//
// Prioritizes a CEO's inbox using a MaxHeap (array/vector-based, built from
// scratch -- no std::priority_queue or other pre-existing heap module).
//
// Priority rules:
//   1. Sender category, highest first:
//        Boss > Subordinate > Peer > ImportantPerson > OtherPerson
//   2. Within the same category, the newest email (by date) is read first.
//
// Commands (read from stdin, one per line):
//   EMAIL <sender category>,<subject line>,<date MM-DD-YYYY>
//   NEXT   - show the next email to be read (does not remove it)
//   READ   - CEO has read/handled the current top email (removes it)
//   COUNT  - display current count of unread emails
//
// Build:  g++ -std=c++17 -O2 -o email_priority email_priority.cpp
// Run:    ./email_priority < testfile.txt

#include <iostream>
#include <sstream> // not used
#include <string>
#include <vector>
#include <cctype> // not uesd

using namespace std;

// ---------------------------------------------------------------------
// Email record
// ---------------------------------------------------------------------
struct Email {
    string sender;      // sender category (Boss, Subordinate, Peer, ...)
    string subject;     // subject/content of the Email
    string date;         // original date string, MM-DD-YYYY
    int    categoryRank; // higher = more important
    long   dateValue;    // comparable form of date, higher = newer // also could be int
};

// Map a sender category string to its priority rank.
// Higher number = higher priority (read first).
int categoryRank(const string& category) {
    if (category == "Boss")           return 5; // If the Email is from boss
    if (category == "Subordinate")    return 4; // If the Email is from Subordinate
    if (category == "Peer")           return 3; // If the Email is from Peer
    if (category == "ImportantPerson")return 2; // If the Email is from an Important Person
    if (category == "OtherPerson")    return 1; // If the Email is from an Other Person
    return 0; // unknown category - lowest priority (shouldn't occur per spec)
}

// Convert MM-DD-YYYY into an integer YYYYMMDD so later dates compare larger.
long dateToValue(const string& date) {
    // Expect exactly "MM-DD-YYYY"
    int mm = 0, dd = 0, yyyy = 0; // initializes the values that we want to return
    // Checks if the input is in the correct formatt and populating the dates value
    // sscanf() is a c function and doesn't accept the c++ formatt for strings so we convert it using .c_str()
    if (sscanf(date.c_str(), "%d-%d-%d", &mm, &dd, &yyyy) == 3) {
        return static_cast<long>(yyyy) * 10000L + mm * 100L + dd; // returns it as a single long
    }
    return 0; // malformed date - treat as oldest
}

// Returns true if 'a' has strictly higher reading priority than 'b'.
bool hasHigherPriority(const Email& a, const Email& b) {
    // Checks if 'a' and 'b' do not have equal category rank
    if (a.categoryRank != b.categoryRank)
        return a.categoryRank > b.categoryRank; // Returns true if 'a' is higher. Otherwise false
    return a.dateValue > b.dateValue; // newer email wins ties
}

// ---------------------------------------------------------------------
// MaxHeap: list(array)-based implementation, built from scratch.
// Root (index 0) always holds the highest-priority email.
// ---------------------------------------------------------------------
class MaxHeap {
public:
    // Insert method for the Maxheap
    void insert(const Email& e) {
        data.push_back(e);  // Pushes a new Email to the end of the vector(array)
        heapifyUp(static_cast<int>(data.size()) - 1); // Upheaps the last element (the one we just added)
    }

    // Remove and return the highest-priority email.
    Email extractMax() {
        Email top = data[0]; // Assingns/Stores top to the first element in vector
        data[0] = data.back(); // Replace the first element with the last
        data.pop_back(); // pop the last element
        // Checks if the vector is not empty
        if (!data.empty())
            heapifyDown(0); // If so downHeap the element we replace the top with
        return top;
    }
    
    // Peek method for the MaxHeap
    const Email& peekMax() const {
        return data[0]; // Just returns the first element in vector
    }

    
    bool empty() const { return data.empty(); } // Method for checking if the vector is empty
    size_t size() const { return data.size(); }  // Method that returns the number of elements in vector

private:
    vector<Email> data; // Declaring the actual dataBase vector for Emails

    static int parent(int i) { return (i - 1) / 2; } // Method that finds the parent of the current element
    static int leftChild(int i) { return 2 * i + 1; } // Method that finds the leftChild of the current element
    static int rightChild(int i) { return 2 * i + 2; } // Method that finds the rightChild of the current element

    // Logic for the UpHeaping. While the current element is bigger than its parent and is not the root, it will keep on swapping with parent.
    void heapifyUp(int i) {
        // Checks if it is bigger than its parent and is not the root
        while (i > 0 && hasHigherPriority(data[i], data[parent(i)])) {
            swap(data[i], data[parent(i)]); // swap places with parent
            i = parent(i); // update the pointer to point at the element we were working with
        }
    }

    // Logic for DownHeaping.
    void heapifyDown(int i) {
        int n = static_cast<int>(data.size()); // Creates a value that stores the size of the vector
        // Loops until a break gets called
        while (true) {
            int largest = i; // Keeps track of the largest element
            int l = leftChild(i);
            int r = rightChild(i);

            if (l < n && hasHigherPriority(data[l], data[largest]))
                largest = l;
            if (r < n && hasHigherPriority(data[r], data[largest]))
                largest = r;

            if (largest == i) break;

            swap(data[i], data[largest]);
            i = largest;
        }
    }
};

// ---------------------------------------------------------------------
// Helpers for parsing input lines
// ---------------------------------------------------------------------
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Split "EMAIL <category>,<subject>,<date>" into its three fields.
// Subject may contain spaces but never commas, so a simple split on
// commas (into exactly 3 parts, first two split, remainder is date) works.
static bool parseEmailLine(const string& rest, Email& outEmail) {
    // rest is everything after "EMAIL " (already trimmed of leading spaces)
    size_t firstComma = rest.find(',');
    if (firstComma == string::npos) return false;

    size_t secondComma = rest.rfind(','); // date has no commas, subject has none either,
                                           // so the LAST comma separates subject from date
    if (secondComma == string::npos || secondComma == firstComma) {
        // exactly one comma found but we need two fields separators;
        // handle case where subject itself might be empty etc.
        secondComma = rest.find(',', firstComma + 1);
        if (secondComma == string::npos) return false;
    }

    string category = trim(rest.substr(0, firstComma));
    string subject  = trim(rest.substr(firstComma + 1, secondComma - firstComma - 1));
    string date     = trim(rest.substr(secondComma + 1));

    outEmail.sender       = category;
    outEmail.subject      = subject;
    outEmail.date         = date;
    outEmail.categoryRank = categoryRank(category);
    outEmail.dateValue    = dateToValue(date);
    return true;
}

// ---------------------------------------------------------------------
// Main: read commands from stdin, drive the MaxHeap
// ---------------------------------------------------------------------
int main() {
    MaxHeap inbox;
    string line;

    while (getline(cin, line)) {
        string trimmed = trim(line);
        if (trimmed.empty()) continue;

        if (trimmed.rfind("EMAIL", 0) == 0) {
            // "EMAIL" followed by a space, then the delimited fields
            string rest = trimmed.substr(5); // strip "EMAIL"
            rest = trim(rest);
            Email e;
            if (parseEmailLine(rest, e)) {
                inbox.insert(e);
            } else {
                cerr << "Warning: could not parse EMAIL line: " << line << endl;
            }
        }
        else if (trimmed == "NEXT") {
            if (inbox.empty()) {
                cout << "Next email:" << endl;
                cout << "    No emails in inbox." << endl;
            } else {
                const Email& e = inbox.peekMax();
                cout << "Next email:" << endl;
                cout << "    Sender: "  << e.sender  << endl;
                cout << "    Subject: " << e.subject << endl;
                cout << "    Date: "    << e.date    << endl;
            }
        }
        else if (trimmed == "READ") {
            if (inbox.empty()) {
                cout << "No emails to read." << endl;
            } else {
                inbox.extractMax();
            }
        }
        else if (trimmed == "COUNT") {
            cout << "There are " << inbox.size() << " emails to read." << endl;
        }
        else {
            cerr << "Warning: unrecognized command: " << line << endl;
        }
    }

    return 0;
}

// gemini

// #include <iostream>
// #include <string>
// #include <sstream>
// #include <vector>
// #include <iomanip>

// struct Email {
//     std::string sender;
//     std::string subject;
//     std::string dateStr; // Format: MM-DD-YYYY
//     int priorityCategory;
//     long long numericDate; // For chronological ordering (YYYYMMDD)

//     // Parse date into YYYYMMDD for easy numerical comparison
//     void parseDate() {
//         int m, d, y;
//         char dash1, dash2;
//         std::stringstream ss(dateStr);
//         ss >> m >> dash1 >> d >> dash2 >> y;
//         numericDate = static_cast<long long>(y) * 10000 + m * 100 + d;
//     }

//     // Determine category weight: Boss (5) > Subordinate (4) > Peer (3) > ImportantPerson (2) > OtherPerson (1)
//     void parsePriority() {
//         if (sender == "Boss") priorityCategory = 5;
//         else if (sender == "Subordinate") priorityCategory = 4;
//         else if (sender == "Peer") priorityCategory = 3;
//         else if (sender == "ImportantPerson") priorityCategory = 2;
//         else priorityCategory = 1; // OtherPerson
//     }

//     // Returns true if this email has HIGHER priority than 'other'
//     bool operator>(const Email& other) const {
//         if (priorityCategory != other.priorityCategory) {
//             return priorityCategory > other.priorityCategory;
//         }
//         // Newer date takes precedence over older date
//         return numericDate > other.numericDate;
//     }
// };

// class MaxHeap {
// private:
//     std::vector<Email> heap;

//     void heapifyUp(int index) {
//         while (index > 0) {
//             int parent = (index - 1) / 2;
//             if (heap[index] > heap[parent]) {
//                 std::swap(heap[index], heap[parent]);
//                 index = parent;
//             } else {
//                 break;
//             }
//         }
//     }

//     void heapifyDown(int index) {
//         int size = heap.size();
//         while (index < size) {
//             int left = 2 * index + 1;
//             int right = 2 * index + 2;
//             int largest = index;

//             if (left < size && heap[left] > heap[largest]) {
//                 largest = left;
//             }
//             if (right < size && heap[right] > heap[largest]) {
//                 largest = right;
//             }

//             if (largest != index) {
//                 std::swap(heap[index], heap[largest]);
//                 index = largest;
//             } else {
//                 break;
//             }
//         }
//     }

// public:
//     void insert(const Email& email) {
//         heap.push_back(email);
//         heapifyUp(heap.size() - 1);
//     }

//     bool peek(Email& result) const {
//         if (heap.empty()) return false;
//         result = heap[0];
//         return true;
//     }

//     bool pop() {
//         if (heap.empty()) return false;
//         heap[0] = heap.back();
//         heap.pop_back();
//         if (!heap.empty()) {
//             heapifyDown(0);
//         }
//         return true;
//     }

//     size_t size() const {
//         return heap.size();
//     }

//     bool empty() const {
//         return heap.empty();
//     }
// };

// // Trim leading whitespace helper
// std::string trim(const std::string& str) {
//     size_t start = str.find_first_not_of(" \t");
//     return (start == std::string::npos) ? "" : str.substr(start);
// }

// int main() {
//     MaxHeap emailQueue;
//     std::string line;

//     while (std::getline(std::cin, line)) {
//         if (line.empty()) continue;

//         if (line.rfind("EMAIL", 0) == 0) {
//             std::string payload = trim(line.substr(5));
//             std::stringstream ss(payload);
//             std::string sender, subject, date;

//             if (std::getline(ss, sender, ',') &&
//                 std::getline(ss, subject, ',') &&
//                 std::getline(ss, date)) {
                
//                 Email email;
//                 email.sender = trim(sender);
//                 email.subject = trim(subject);
//                 email.dateStr = trim(date);
//                 email.parsePriority();
//                 email.parseDate();

//                 emailQueue.insert(email);
//             }
//         } 
//         else if (line == "COUNT") {
//             std::cout << "There are " << emailQueue.size() << " emails to read." << std::endl;
//         } 
//         else if (line == "NEXT") {
//             Email nextEmail;
//             if (emailQueue.peek(nextEmail)) {
//                 std::cout << "Next email:" << std::endl;
//                 std::cout << "    Sender: " << nextEmail.sender << std::endl;
//                 std::cout << "    Subject: " << nextEmail.subject << std::endl;
//                 std::cout << "    Date: " << nextEmail.dateStr << std::endl;
//             }
//         } 
//         else if (line == "READ") {
//             emailQueue.pop();
//         }
//     }

//     return 0;
// }