#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

// ============================================================================
// CLASS: PrologDatabase
// Purpose: Stores PROLOG facts and rules, and provides querying functionality
// ============================================================================
class PrologDatabase {
private:
    // Storage for facts: predicate name -> list of argument tuples
    // Example: "parent" -> [["john", "mary"], ["mary", "susan"]]
    map<string, vector<vector<string>>> facts;
    
    // Helper function to convert string to lowercase for case-insensitive matching
    string toLower(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    // Helper function to trim whitespace from both ends of a string
    string trim(const string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        
        if (start == string::npos) return "";
        return str.substr(start, end - start + 1);
    }

public:
    // ------------------------------------------------------------------------
    // METHOD: addFact
    // Purpose: Adds a new fact to the database
    // Parameters:
    //   - predicate: The name of the predicate (e.g., "parent", "likes")
    //   - arguments: Vector of arguments for this predicate
    // ------------------------------------------------------------------------
    void addFact(const string& predicate, const vector<string>& arguments) {
        string pred = toLower(predicate);
        
        // Add the fact to our database
        facts[pred].push_back(arguments);
        
        // Print confirmation for user
        cout << "Added fact: " << predicate << "(";
        for (size_t i = 0; i < arguments.size(); i++) {
            cout << arguments[i];
            if (i < arguments.size() - 1) cout << ", ";
        }
        cout << ")" << endl;
    }
    
    // ------------------------------------------------------------------------
    // METHOD: query
    // Purpose: Queries the database for facts matching the given predicate
    // Parameters:
    //   - predicate: The predicate to search for
    //   - arguments: Arguments to match (use "?" for wildcards/variables)
    // Returns: Vector of all matching fact argument lists
    // ------------------------------------------------------------------------
    vector<vector<string>> query(const string& predicate, 
                                  const vector<string>& arguments) {
        vector<vector<string>> results;
        string pred = toLower(predicate);
        
        // Check if this predicate exists in our database
        if (facts.find(pred) == facts.end()) {
            return results; // Empty results
        }
        
        // Search through all facts with this predicate
        for (const auto& fact : facts[pred]) {
            bool matches = true;
            
            // Check if the number of arguments matches
            if (fact.size() != arguments.size()) {
                matches = false;
                continue;
            }
            
            // Check each argument
            for (size_t i = 0; i < arguments.size(); i++) {
                // "?" is a wildcard that matches anything
                if (arguments[i] != "?" && 
                    toLower(arguments[i]) != toLower(fact[i])) {
                    matches = false;
                    break;
                }
            }
            
            if (matches) {
                results.push_back(fact);
            }
        }
        
        return results;
    }
    
    // ------------------------------------------------------------------------
    // METHOD: printDatabase
    // Purpose: Displays all facts currently stored in the database
    // ------------------------------------------------------------------------
    void printDatabase() {
        cout << "\n========== PROLOG DATABASE ==========\n";
        
        if (facts.empty()) {
            cout << "Database is empty.\n";
            return;
        }
        
        // Iterate through all predicates
        for (const auto& predicate : facts) {
            cout << "\nPredicate: " << predicate.first << endl;
            
            // Print all facts for this predicate
            for (const auto& fact : predicate.second) {
                cout << "  " << predicate.first << "(";
                for (size_t i = 0; i < fact.size(); i++) {
                    cout << fact[i];
                    if (i < fact.size() - 1) cout << ", ";
                }
                cout << ")" << endl;
            }
        }
        
        cout << "=====================================\n\n";
    }
};

// ============================================================================
// CLASS: TextParser
// Purpose: Converts natural language sentences into PROLOG predicates
// ============================================================================
class TextParser {
private:
    PrologDatabase& db;
    
    // Helper function to split a string by delimiter
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        
        while (getline(ss, token, delimiter)) {
            // Trim whitespace from token
            token.erase(0, token.find_first_not_of(" \t\n\r"));
            token.erase(token.find_last_not_of(" \t\n\r") + 1);
            
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        
        return tokens;
    }
    
    // Helper function to convert string to lowercase
    string toLower(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    // Helper function to remove punctuation from end of string
    string removePunctuation(string str) {
        while (!str.empty() && ispunct(str.back())) {
            str.pop_back();
        }
        return str;
    }
    
    // ------------------------------------------------------------------------
    // METHOD: parseRelationship
    // Purpose: Parse sentences expressing relationships
    // Example: "John is the parent of Mary" -> parent(john, mary)
    // ------------------------------------------------------------------------
    void parseRelationship(const vector<string>& words) {
        // Look for common relationship patterns
        // Pattern 1: "X is the RELATION of Y"
        for (size_t i = 0; i < words.size(); i++) {
            string word = toLower(words[i]);
            
            if (word == "is" && i + 3 < words.size() && 
                toLower(words[i+1]) == "the" && 
                toLower(words[i+3]) == "of") {
                
                string subject = removePunctuation(toLower(words[i-1]));
                string relation = removePunctuation(toLower(words[i+2]));
                string object = removePunctuation(toLower(words[i+4]));
                
                db.addFact(relation, {subject, object});
                return;
            }
        }
        
        // Pattern 2: "X RELATION Y" (e.g., "John likes Mary")
        if (words.size() >= 3) {
            string subject = removePunctuation(toLower(words[0]));
            string relation = removePunctuation(toLower(words[1]));
            string object = removePunctuation(toLower(words[2]));
            
            db.addFact(relation, {subject, object});
        }
    }
    
    // ------------------------------------------------------------------------
    // METHOD: parseProperty
    // Purpose: Parse sentences expressing properties/attributes
    // Example: "John is tall" -> tall(john)
    // ------------------------------------------------------------------------
    void parseProperty(const vector<string>& words) {
        if (words.size() >= 3 && toLower(words[1]) == "is") {
            string subject = removePunctuation(toLower(words[0]));
            string property = removePunctuation(toLower(words[2]));
            
            // Check if it's a property (adjective) or a noun
            // For simplicity, we treat everything after "is" as a property
            db.addFact(property, {subject});
        }
    }
    
    // ------------------------------------------------------------------------
    // METHOD: parseLivesIn
    // Purpose: Parse location-based sentences
    // Example: "John lives in Paris" -> lives_in(john, paris)
    // ------------------------------------------------------------------------
    void parseLivesIn(const vector<string>& words) {
        for (size_t i = 0; i < words.size(); i++) {
            if (toLower(words[i]) == "lives" && i + 2 < words.size() &&
                toLower(words[i+1]) == "in") {
                
                string subject = removePunctuation(toLower(words[i-1]));
                string location = removePunctuation(toLower(words[i+2]));
                
                db.addFact("lives_in", {subject, location});
                return;
            }
        }
    }

public:
    // Constructor: Initialize with a reference to the database
    TextParser(PrologDatabase& database) : db(database) {}
    
    // ------------------------------------------------------------------------
    // METHOD: parseText
    // Purpose: Main parsing function - determines sentence type and calls
    //          appropriate parsing method
    // Parameters:
    //   - text: The natural language sentence to parse
    // ------------------------------------------------------------------------
    void parseText(const string& text) {
        cout << "\nParsing: \"" << text << "\"" << endl;
        
        // Split the text into words
        vector<string> words = split(text, ' ');
        
        if (words.empty()) {
            cout << "Empty sentence, nothing to parse.\n";
            return;
        }
        
        // Determine the type of sentence and parse accordingly
        string textLower = toLower(text);
        
        // Check for "lives in" pattern
        if (textLower.find("lives in") != string::npos) {
            parseLivesIn(words);
        }
        // Check for "is the ... of" pattern (relationships)
        else if (textLower.find("is the") != string::npos && 
                 textLower.find(" of ") != string::npos) {
            parseRelationship(words);
        }
        // Check for simple "is" pattern (properties or relationships)
        else if (textLower.find(" is ") != string::npos) {
            // Try to determine if it's a property or relationship
            if (words.size() == 3) {
                parseProperty(words);
            } else {
                parseRelationship(words);
            }
        }
        // Default: assume it's a simple relationship
        else if (words.size() >= 3) {
            parseRelationship(words);
        }
        else {
            cout << "Could not parse sentence pattern.\n";
        }
    }
};

// ============================================================================
// CLASS: QueryEngine
// Purpose: Processes natural language queries and retrieves answers from DB
// ============================================================================
class QueryEngine {
private:
    PrologDatabase& db;
    
    // Helper function to convert string to lowercase
    string toLower(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    // Helper function to remove punctuation
    string removePunctuation(string str) {
        while (!str.empty() && ispunct(str.back())) {
            str.pop_back();
        }
        return str;
    }

public:
    // Constructor
    QueryEngine(PrologDatabase& database) : db(database) {}
    
    // ------------------------------------------------------------------------
    // METHOD: processQuery
    // Purpose: Converts a natural language question into a PROLOG query
    // Parameters:
    //   - question: Natural language question to process
    // ------------------------------------------------------------------------
    void processQuery(const string& question) {
        cout << "\nQuery: \"" << question << "\"" << endl;
        
        string questionLower = toLower(question);
        
        // Pattern 1: "Who is the RELATION of X?"
        if (questionLower.find("who is the") != string::npos) {
            size_t pos = questionLower.find("of ");
            if (pos != string::npos) {
                string relation = extractRelation(questionLower);
                string object = extractObject(questionLower, pos + 3);
                
                // Query with wildcard for subject
                auto results = db.query(relation, {"?", object});
                printResults(results, "Who", 0);
            }
        }
        // Pattern 2: "What does X RELATION?"
        else if (questionLower.find("what does") != string::npos) {
            size_t pos = questionLower.find("what does ");
            string rest = questionLower.substr(pos + 10);
            
            vector<string> words;
            stringstream ss(rest);
            string word;
            while (ss >> word) {
                words.push_back(removePunctuation(word));
            }
            
            if (words.size() >= 2) {
                string subject = words[0];
                string relation = words[1];
                
                auto results = db.query(relation, {subject, "?"});
                printResults(results, "Answer", 1);
            }
        }
        // Pattern 3: "Where does X live?"
        else if (questionLower.find("where does") != string::npos &&
                 questionLower.find("live") != string::npos) {
            size_t pos = questionLower.find("where does ");
            string rest = questionLower.substr(pos + 11);
            
            stringstream ss(rest);
            string subject;
            ss >> subject;
            subject = removePunctuation(toLower(subject));
            
            auto results = db.query("lives_in", {subject, "?"});
            printResults(results, "Location", 1);
        }
        // Pattern 4: "Is X PROPERTY?" or "Is X RELATION Y?"
        else if (questionLower.find("is ") == 0) {
            vector<string> words;
            stringstream ss(questionLower);
            string word;
            while (ss >> word) {
                words.push_back(removePunctuation(word));
            }
            
            if (words.size() == 3) { // "Is X PROPERTY?"
                string subject = words[1];
                string property = words[2];
                
                auto results = db.query(property, {subject});
                if (!results.empty()) {
                    cout << "Answer: Yes\n";
                } else {
                    cout << "Answer: No (or unknown)\n";
                }
            } else if (words.size() >= 4) { // "Is X RELATION Y?"
                string subject = words[1];
                string relation = words[2];
                string object = words[3];
                
                auto results = db.query(relation, {subject, object});
                if (!results.empty()) {
                    cout << "Answer: Yes\n";
                } else {
                    cout << "Answer: No (or unknown)\n";
                }
            }
        }
        else {
            cout << "Could not understand query format.\n";
        }
    }
    
private:
    // Helper function to extract relation name from question
    string extractRelation(const string& question) {
        size_t start = question.find("is the ") + 7;
        size_t end = question.find(" of", start);
        
        if (start != string::npos && end != string::npos) {
            return removePunctuation(question.substr(start, end - start));
        }
        return "";
    }
    
    // Helper function to extract object from question
    string extractObject(const string& question, size_t startPos) {
        string rest = question.substr(startPos);
        stringstream ss(rest);
        string object;
        ss >> object;
        return removePunctuation(object);
    }
    
    // Helper function to print query results
    void printResults(const vector<vector<string>>& results, 
                     const string& label, size_t index) {
        if (results.empty()) {
            cout << "Answer: No matches found.\n";
        } else {
            cout << label << ":\n";
            for (const auto& result : results) {
                if (index < result.size()) {
                    cout << "  - " << result[index] << endl;
                }
            }
        }
    }
};

// ============================================================================
// MAIN FUNCTION
// Purpose: Demonstrates the PROLOG text parser with example data and queries
// ============================================================================
int main() {
    cout << "========================================\n";
    cout << "   PROLOG TEXT PARSER IN C++\n";
    cout << "========================================\n\n";
    
    // Create the PROLOG database
    PrologDatabase prologDB;
    
    // Create parser and query engine
    TextParser parser(prologDB);
    QueryEngine queryEngine(prologDB);
    
    // =========================================================================
    // STEP 1: Parse natural language statements and add to database
    // =========================================================================
    cout << "STEP 1: Parsing natural language statements\n";
    cout << "--------------------------------------------\n";
    
    // Parse family relationships
    parser.parseText("John is the parent of Mary");
    parser.parseText("Mary is the parent of Susan");
    parser.parseText("John is the parent of Tom");
    parser.parseText("Tom is the parent of Alice");
    
    // Parse friendships
    parser.parseText("John likes pizza");
    parser.parseText("Mary likes chocolate");
    parser.parseText("Susan likes music");
    
    // Parse locations
    parser.parseText("John lives in Paris");
    parser.parseText("Mary lives in London");
    parser.parseText("Susan lives in Tokyo");
    
    // Parse properties
    parser.parseText("Alice is tall");
    parser.parseText("Tom is smart");
    
    // =========================================================================
    // STEP 2: Display the database contents
    // =========================================================================
    prologDB.printDatabase();
    
    // =========================================================================
    // STEP 3: Process natural language queries
    // =========================================================================
    cout << "\nSTEP 2: Processing queries\n";
    cout << "--------------------------------------------\n";
    
    // Query 1: Who is the parent of someone?
    queryEngine.processQuery("Who is the parent of Mary?");
    
    // Query 2: What does someone like?
    queryEngine.processQuery("What does John like?");
    
    // Query 3: Where does someone live?
    queryEngine.processQuery("Where does Mary live?");
    
    // Query 4: Is someone a property?
    queryEngine.processQuery("Is Alice tall?");
    
    // Query 5: Is someone related to someone else?
    queryEngine.processQuery("Is John the parent of Tom?");
    
    // Query 6: Find all children of John
    queryEngine.processQuery("Who is the parent of Susan?");
    
    // =========================================================================
    // STEP 4: Demonstrate direct database queries (PROLOG-style)
    // =========================================================================
    cout << "\n\nSTEP 3: Direct PROLOG-style queries\n";
    cout << "--------------------------------------------\n";
    
    cout << "\nQuery: parent(?, mary) - Find all parents of Mary\n";
    auto results1 = prologDB.query("parent", {"?", "mary"});
    for (const auto& result : results1) {
        cout << "  Result: " << result[0] << endl;
    }
    
    cout << "\nQuery: parent(john, ?) - Find all children of John\n";
    auto results2 = prologDB.query("parent", {"john", "?"});
    for (const auto& result : results2) {
        cout << "  Result: " << result[1] << endl;
    }
    
    cout << "\nQuery: lives_in(?, ?) - Find all living arrangements\n";
    auto results3 = prologDB.query("lives_in", {"?", "?"});
    for (const auto& result : results3) {
        cout << "  Result: " << result[0] << " lives in " << result[1] << endl;
    }
    
    cout << "\n========================================\n";
    cout << "   Program completed successfully!\n";
    cout << "========================================\n";
    
    return 0;
}
