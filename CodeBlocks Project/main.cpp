//OS based includes for the fullscreen implementation
#ifdef _WIN32
    #define NOMINMAX
  // This is to prevent windows.h from defining min and max macros that can interfere with std::min and std::max
  // because we're using namespace std, which is considered bad practice but for the
  // sake of simplicity and readability in this project,
  // we will use it, so we need to undefine these macros to avoid conflicts with our code
    #define WIN32_LEAN_AND_MEAN

  // we need this header for windows API functions to get the console size for our fullscreen implementation
    #include <windows.h>

  // undefined these macros that are defined in windows.h to avoid conflicts with our enum values
    #undef DELETE
    #undef SEARCH
    #undef EXIT
    #undef EDIT
    #undef RETURN
#else
  // for Unix-like systems, we need these headers to get the console size for our fullscreen implementation
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

//================================================================================================================

#include <iostream>
#include <string>
#include <vector> // to use the dynamic array vector
#include <fstream> // for file handling
#include <sstream> // for stringstream to parse the file
#include <iomanip> // for setw to format the output
#include <algorithm> // for transform to lowercase the input
#include <ctime>

using namespace std;

struct Book {
  string title;
  string author;
  string category;
  string genre;
  int year;
  int ID;
  bool isAvailable;
  string dateAdded;

  // for borrowing
  string dateBorrowed;
  string borrowerName;
  string dueDate;
};


struct Genre {
const vector<string> Fiction = {
    "Fantasy",
    "SciFi",
    "Mystery",
    "Thriller",
    "Romance",
    "Horror",
    "Adventure",
    "Drama",
    "YoungAdult",
    "Dystopian",
    "HistoricalFiction",
    "Crime",
    "Action",
    "SliceOfLife",
    "Other"
};

const vector<string> nonFiction = {
    "Biography",
    "Autobiography",
    "Memoir",
    "History",
    "Science",
    "Technology",
    "Philosophy",
    "Psychology",
    "Economics",
    "Education",
    "Health",
    "Travel",
    "Religion",
    "Politics",
    "TrueCrime",
    "Other"
};

};

enum MENU {
  ADD=1,
  EDIT,
  DELETE,
  SEARCH,
  BORROW,
  RETURN,
  VIEWALL,
  EXIT,
  HOME,
  FILTER
};



// Forward Declaration of our Main Functions
void AddBook(vector<Book>&);
void EditBook(vector<Book>&);
void DeleteBook(vector<Book>&);
void SearchBook(vector<Book>&);
void BorrowBook(vector<Book>&);
void ReturnBook(vector<Book>&);
void ViewAllBook(vector<Book>&, MENU); // viewer/controller
void Exit();

// Forward Declaration of our Helper Functions
void LoadFile(vector<Book>&); // load a txt file
void SaveFile(vector<Book>&); // save a file
void Log(string); // nag lolog
void Clear(); // clear the screen
void FilterBooks(vector<Book>&); // filter books by certain criteria
void ShowFullBookDetails(vector<Book>&); // since table form can limit readability, implemented full view of details
void ListAllBooks(vector<Book>&, int); // list lahat buks
void ListGenre(string); // helper to pick genre, returns the corresponding enum value
string ToLowerWord(string); // to make inputs lowercased
string GetDate(int); // date
string GetTime(); // time
time_t ParseDate(string); // cuz I dont wanna deal with allat time_t shenanigans, cant getline time_t to file
int GetID(vector<Book>&); // Get the latest ID then ++
int GetFine(string); // Fine System lesgaw

// Forwared Declaration of our Validators
bool isNumber(const string&); // for fields that requires numbers only
bool isYearValid(string); // for year, currently capped at 2026
bool isGenreValid(string, string&, string&); // We used predefined Genres for data consistency
bool isCategoryValid(string&, string&); // 1 or 2

// Forward Declaration of our UI helpers
string green(string); //color
string red(string); //kolor
string yellow(string); //kulay
string cyan(string); //crayon
string purple(string); //pupol
string orange(string); //kahel
string bold(string); //uy
string fitTextWidth(string, int); // truncator ...
string stripAnsi(string); // freakin ANSI escape codes are invisible to .length() annoying
string Highlight(string, string); // hehe meron kaming highlight feature
void getScreenSize(int&, int&); //Fullscreen implementation
void RefreshUI(MENU); // Reprint screen when screen size changed
void RefreshMainScreen(); // the main box refresh
void DrawUI(); // All UI elements in one
void DrawHeader(); // mainly title
void DrawFooter(); // legends, controls, input, status, etc
void DrawMainScreen(); // just a big box
void DrawTextBox(int, int, int, bool, string); // I MADE THIS RAHHHHHHHHH, I USED MATH
void ResetCursorToInput(); // put the cursor always below the screen
void SetCursorIn(int, int); // cursor pointer pointing to pointer
void PrintLegends(MENU); // Print Options
void PrintState(MENU); // Aethethic I guess
void PrintLine(string, string, string); // helper to print lines with the box drawing characters, used for the header and footer
void PrintTableHeaders(); // Book data on TOP
void PrintBookDetails(bool, const Book&); // helper to print the details of a book in a formatted way, used for the view all function
void PrintPage(int&, int&); // We HAVE Pagination page 1 of 10000000
void UpdateStatus(string); // Personality ;^)


// Theming weheheheheheh (GRUVBOX)
auto bg = []()  { return "\x1b[48;2;40;40;40m\x1b[38;2;146;131;116m"; };
auto txtcol = []() { return "\x1b[38;2;253;244;193m"; };
auto legcol = []() { return "\x1b[38;2;213;196;161m"; };




// ewan
string gold_line(string t) { return "\x1b[38;2;215;153;33m" + t + "\x1b[0m"; } // #d79921}



// For full screen UI, set globally for convenience and readability
// No jumbled spagetti code here, just a simple getter for the screen size, and we can use these variables anywhere in the code
int consoleWidth, consoleHeight, prevW=0, prevH=0;

// due to my lack of knowledge about time_t I made my own workaround
// these are for Dates. Fine system
bool TEST_DATE = false; // flip this if you want to test fine system along with time advancement, "time_t now" is not affected by int today
int today = 0; // change the value of that for testing, days. 0 is today, toggle TEST_DATE to test fine system
int setDueDate = 7; // this sets the due date of books in days, changing this will only affect newly borrowed books


int main() {
  vector<Book> Library;
  LoadFile(Library);

  Log("Library Opened!");
  Log("Loaded " + to_string(Library.size()) + " Books!");

  while (true) {
    Clear();
    getScreenSize(consoleWidth, consoleHeight);
    DrawUI();
    PrintState(HOME);
    UpdateStatus(cyan("Welcome to the Library of Alexandria!"));

    string input;
    getline(cin, input);

    // validate the input here, if its empty, redo
    if (input.empty()) {
        continue;
    }

    // if it's not empty, check if its a digit
    int choice;
    if (isNumber(input)) {
        choice = stoi(input); // convert that string into an int to use for enums
    } else {
        UpdateStatus(red("Please enter a number corresponding to the menu options."));
        cin.get(); // if it's not a digit, redo again
        continue;
    }

    switch (choice) {
    // choices    |      functions               |         update        |   break
      case ADD:       AddBook(Library);               SaveFile(Library);     break;
      case EDIT:      EditBook(Library);              SaveFile(Library);     break;
      case DELETE:    DeleteBook(Library);            SaveFile(Library);     break;
      case SEARCH:    SearchBook(Library);                                   break;
      case BORROW:    BorrowBook(Library);            SaveFile(Library);     break;
      case RETURN:    ReturnBook(Library);            SaveFile(Library);     break;
      case VIEWALL:   ViewAllBook(Library, VIEWALL);                         break;
      case EXIT:      Exit();                                                break;

      default:
      UpdateStatus(red("Invalid choice! Please enter a number between 1 and 8."));
      cin.get();
    }
    Clear();
  }

  return 0;
}



//===========================================================================================================
// Main Functions here

// some random ahh adlib
auto showHint = []() {
  SetCursorIn(3, consoleHeight-8);
  cout << yellow("Tip: ") << bg() << cyan("Go to [") << bg() << yellow("ViewAll") << bg() << cyan("] first for the ID");
};

void AddBook(vector<Book>& Library){
RefreshUI(ADD);
PrintState(ADD);
PrintLegends(ADD);
UpdateStatus(cyan("Adding a new book..."));

    Book b; // let it b

    // keeping all things organized and automated so we avoid repeating codes
    // might've sacrificed readability tho
    struct Field {
    string label;
    string value;
    };

    vector<Field> fields = {
    {"Enter Title", ""},
    {"Enter Author", ""},
    {"Enter Year", ""},
    {"Choose Category", ""},
    {"Choose Genre", ""}
    };

    // uhh this might be good for readablity
    enum FieldIndex {
        TITLE_FIELD = 0,
        AUTHOR_FIELD,
        YEAR_FIELD,
        CATEGORY_FIELD,
        GENRE_FIELD
    };

    string pickedGenre;
    string pickedCategory;
    string selCat;

    for (int i = 0; i < fields.size(); i++) {
    // this will loop until the inputs are valid
    while (true) {
    RefreshUI(ADD);

    // Display thingies for specific fields
    if (i == CATEGORY_FIELD) {
        DrawTextBox(50, 0, 5, true, yellow("1") + bg() + cyan(" for Fiction | ")
                    + bg() + bold(yellow("2")) + bg() + cyan(" for Non-Fiction"));
    }
    if (i == GENRE_FIELD) {
        ListGenre(fields[3].value);
    }

    // Main action area
    DrawTextBox(40, 0, 0, true, cyan(fields[i].label));
    ResetCursorToInput();
    getline(cin, fields[i].value);


    // Validator Fields
    if (fields[i].value.empty()) {
        UpdateStatus(red("Input cannot be empty!"));
        continue;
    }

    if (ToLowerWord(fields[i].value) == "/cancel") {
      DrawTextBox(40, 0, 0, true, red("CANCELLED"));
      UpdateStatus(red("Cancelled Adding Book, bruh"));
      cin.get();
      return;
    }

    switch (i) {
    case TITLE_FIELD:
      UpdateStatus(green("Book's Title is set to '") + bg() + yellow(fields[i].value) + bg() + green("'"));
     break;
    case AUTHOR_FIELD:
      UpdateStatus(green("Book's Author is set to '") + bg() + yellow(fields[i].value) + bg() + green("'"));
      break;
    case YEAR_FIELD:
      if (!isYearValid(fields[2].value)) { continue; }
      break;
    case CATEGORY_FIELD:
      if (!isCategoryValid(fields[3].value, pickedCategory)) { continue; };
      break;
    case GENRE_FIELD:
      if (!isGenreValid(fields[4].value, pickedCategory, pickedGenre)) { continue; };
      break;

      default: continue; // redo
  }
    break; // exit this validating loop
  }
  }


  // assign those values to the book struct
    b.title = fields[0].value;
    b.author = fields[1].value;
    b.year = stoi(fields[2].value); //convert it to int cuz iz string
    b.category = pickedCategory;
    b.genre = pickedGenre; // send this to the genre picker to assign
    b.ID = GetID(Library); // goated ID assignment based on the last added
    b.isAvailable = true; // set the availability status to true when adding a new book
    b.dateAdded = GetDate(0);

    // Preview what they added
    Library.push_back(b); // keep this as this will be the updater of our array
    RefreshMainScreen();
    PrintTableHeaders();
    SetCursorIn(3, 7);
    PrintBookDetails(0, Library.back()); // print the details of the last added book, which is the one we just added, to confirm that it was added successfully

    // Confirm if they want to add that book, if not it will be removed from the library vector
    DrawTextBox(50, 0, 2, true, yellow("Do you want to add this book? (Y/n)"));
    ResetCursorToInput();
    if (cin.get() == tolower('n')) {
        Library.pop_back(); // remove the last added book if they choose not to add it
        DrawTextBox(50, 0, 2, true, red("Aww... I want books :("));
        UpdateStatus(red("Cancelled adding book..."));
        cin.get();
        cin.ignore();
        return;
    }
        Log("Book Added: " + b.title);
        DrawTextBox(50, 0, 2, true, green("Thank You for Donating!"));
        UpdateStatus(green("Book added successfully, yey!"));
        cin.get();
        cin.ignore(1000, '\n');
}



void EditBook(vector<Book>& Library) {
RefreshUI(EDIT);
PrintState(EDIT);
PrintLegends(EDIT);
UpdateStatus(cyan("Editing a book..."));
PrintTableHeaders();
if (Library.size() == 0) {
  DrawTextBox(50, 0, 0, true, red("NO BOOKS IN HERE"));
  UpdateStatus(red("Nothing to edit here brotatochip...."));
  cin.get();
  return;
}
showHint();

  DrawTextBox(50, 0, 0, true, cyan("Enter Book ID"));
  ResetCursorToInput();

  // hey some variables here
  bool found = false;
  string tempId;
  int ID;


auto updateField = [](string& field, string prompt) {
    RefreshUI(EDIT);
    DrawTextBox(50, 0, 0, true, cyan(prompt));
    ResetCursorToInput();

    string input;
    getline(cin, input);

    if (input.empty()) {
      UpdateStatus(red("No input... Action cancelled"));
      return false;
    }

    field = input;
    return true;
};

auto updateCatGen = [](string& category, string& genre) {
    RefreshUI(EDIT);
    DrawTextBox(50, 0, 5, true, yellow("1") + bg() + cyan(" for Fiction | ")
                + bg() + bold(yellow("2")) + bg() + cyan(" for Non-Fiction"));
    ResetCursorToInput();

    string pickedCategory;
    string pickedGenre;
    string selCat;
    string selGen;

    string cat;
    getline(cin, cat);

    if (cat.empty()) {
    UpdateStatus(red("No input... Action cancelled"));
    return false;
    }

    if (isCategoryValid(cat, pickedCategory)) {
      selCat = pickedCategory;
    } else { UpdateStatus(red("Invalid Choice...")); return false; }

    ListGenre(cat);
    DrawTextBox(50, 0, 0, true, cyan("Choose Genre"));
    ResetCursorToInput();

    string gen;
    getline(cin, gen);

    if (gen.empty()) {
    UpdateStatus(red("No input... Action cancelled"));
    return false;
    }

    if (isGenreValid(gen, pickedCategory, pickedGenre)) {
      selGen = pickedGenre;
    } else { UpdateStatus(red("Invalid Choice...")); return false; }

    category = selCat;
    genre = selGen;
    RefreshMainScreen();
    return true;
};

  getline(cin, tempId);

  if (tempId.empty()) {
    UpdateStatus(red("No input... cancelled. Going back..."));
    cin.get();
    return;
  }

  if (isNumber(tempId)) {
    ID = stoi(tempId);
  } else {
    DrawTextBox(50, 0, 0, true, red("INVALID BOOK ID"));
    UpdateStatus(red("There is no such ID, maybe try with only numbers..."));
    cin.get();
    return;
  }



    // Main Action Area
    for (Book& b : Library) {
    RefreshUI(EDIT);
    if (b.ID == ID) {
      found = true;
      Book temp = b; // temporary first cuz we gon confirm in the end
      UpdateStatus(green("Book Found!"));

      while (true) {
      RefreshUI(EDIT);
      PrintTableHeaders();
      SetCursorIn(3, 7);
      PrintBookDetails(0, temp);

      DrawTextBox(50, 0, 0, true, cyan("Choose Field to Edit"));
      ResetCursorToInput();
      char choice = tolower(cin.get());
      cin.ignore(1000, '\n');

      if (choice == '\n') {
      UpdateStatus(red("Choose from 1 - 4."));
      continue;
      }

      string tempYear = to_string(temp.year);
      switch (choice) {
        case '1': UpdateStatus(cyan("Editing the title..."));
                  if(updateField(temp.title, "Change Title to?"))
                  {  UpdateStatus(green("Changed Title!")); };
                  break;

        case '2':  UpdateStatus(cyan("Editing the author..."));
                  if (updateField(temp.author, "Change Author to?"))
                  { UpdateStatus(green("Changed Author!")); };
                  break;

        case '3':   while (true) {
                      UpdateStatus(cyan("Editing the year..."));
                      updateField(tempYear, "Change Year to?");
                      if (isYearValid(tempYear)) {
                      temp.year = stoi(tempYear);
                      if (temp.year == b.year) { UpdateStatus(red("No Changes Made in Year")); } else {
                      UpdateStatus(green("Changed Year: '" + yellow(to_string(b.year)) + bg() + green("' into '") + bg() + yellow(tempYear) + bg() + green("'")));
                      }
                      break;
                      }
                    }
                    break;

        case '4': UpdateStatus(cyan("Editing the Category&Genre..."));
                  if (updateCatGen(temp.category, temp.genre))
                  { UpdateStatus(green("Changed Category&Genre!")); };
                  break;

        default:
        UpdateStatus(red("Please choose one of the fields..."));
        cin.ignore(1000, '\n');
        continue;
        break;
      }
      RefreshUI(EDIT);

      PrintTableHeaders();
      SetCursorIn(3, 7);
      PrintBookDetails(0, temp);

      DrawTextBox(50, 0, 0, true, yellow("Continue Editing? (y/N)"));
      ResetCursorToInput();

      if (tolower(cin.get()) != 'y') {
        cin.ignore(1000, '\n');
        UpdateStatus(cyan("Leaving edit mode..."));
        break;
      }
    }

      if (b.title == temp.title && b.author == temp.author &&
          b.year == temp.year && b.category == temp.category &&
          b.genre == temp.genre ) {
          DrawTextBox(50, 0, 0, true, red("NO CHANGES MADE!"));
          UpdateStatus(yellow("You edited NONE!"));
          cin.get();
          return;
          }

      DrawTextBox(50, 0, 0, true, yellow("Save changes? (y/N)"));
      ResetCursorToInput();

      if (tolower(cin.get()) == 'y') {
      cin.ignore(1000, '\n');
        if (b.title != temp.title)        { Log("Edited Book ID s" + to_string(b.ID) + " Title '"    + b.title   + "' into '" + temp.title + "'"); }
        if (b.author != temp.author)      { Log("Edited Book ID s" + to_string(b.ID) + " Author '"   + b.author  + "' into '" + temp.author + "'"); }
        if (b.year != temp.year)          { Log("Edited Book ID s" + to_string(b.ID) + " Year '"     + to_string(b.year) + "' into '" + to_string(temp.year) + "'"); }
        if (b.category != temp.category)  { Log("Edited Book ID s" + to_string(b.ID) + " Category '" + b.category + "' into '" + temp.category + "'"); }
        if (b.genre != temp.genre)        { Log("Edited Book ID s" + to_string(b.ID) + " Genre '"    + b.genre + "' into '" + temp.genre + "'"); }
        b = temp;
        DrawTextBox(50, 0, 0, true, green("CHANGES SAVED!"));
        UpdateStatus(green("Changes has been saved. Exitting..."));
        cin.get();
        cin.ignore(1000, '\n');
        return;
      }
        cin.ignore(1000, '\n');
        DrawTextBox(50, 0, 0, true, red("NO CHANGES MADE!"));

        UpdateStatus(red("Bro said 'No'. Exitting..."));
        cin.get();
        cin.ignore(1000, '\n');
        return;
  }
}
      if (!found) {
      DrawTextBox(50, 0, 0, true, red("BOOK NOT FOUND"));
      ResetCursorToInput();
      UpdateStatus(red("ID not found :("));
      cin.get();
      }
}



void DeleteBook(vector<Book>& Library) {
RefreshUI(DELETE);
PrintState(DELETE);
UpdateStatus(cyan("Deleting a book..."));
if (Library.size() == 0) {
  DrawTextBox(50, 0, 0, true, red("NO BOOKS TO DELETE"));
  UpdateStatus(red("Nothing to delete bruh, add sumn...."));
  cin.get();
  return;
}

showHint();
    DrawTextBox(30, 0, 0, true, cyan("Enter ID to 'DELETE'"));
    ResetCursorToInput();

    bool found = false;

    string tempId;
    getline(cin, tempId);

    if (tempId.empty()) {
      UpdateStatus(red("No input... going back... phew..."));
      cin.get();
      return;
    }

    int ID;
    if (isNumber(tempId)) {
      ID = stoi(tempId);
    } else {
      DrawTextBox(50, 0, 0, true, red("INVALID BOOK ID"));
      UpdateStatus(red("No ID found. wehehehe..."));
      cin.get();
      return;
    }


    for (size_t i = 0; i < Library.size(); i++) {
      if (ID == Library[i].ID) {
      while (true) {
      found = true;
      RefreshUI(EDIT);
      PrintTableHeaders();
      SetCursorIn(3, 7);
      PrintBookDetails(0, Library[i]);
      UpdateStatus(green("uhhh... Book Found..."));

      DrawTextBox(40, 0, 0, true, cyan("Delete this Book..?"));
      DrawTextBox(40, 0, 5, true, yellow("(Y)es | (N)o"));
      ResetCursorToInput();

      char choice = tolower(cin.get());
      cin.ignore(1000, '\n');

      if (choice == 'y') {
      DrawTextBox(40, 0, 0, true, cyan("Confirm Deletion"));
      DrawTextBox(40, 0, 5, true, yellow("Type: 'delete' to confirm"));
      UpdateStatus(cyan("Waiting for confirmation..."));
      string conf;
      getline(cin, conf);
        if (conf == "delete") {
          Log("Deleted Book ID " + to_string(Library[i].ID));
          Library.erase(Library.begin() + i);
          RefreshMainScreen();
          DrawTextBox(40, 0, 0, true, green("book gone... :("));
          UpdateStatus(green("Book has been successfully Vaporized."));
          cin.get();
          return;
        }
      }


      if (choice == 'n') {
          RefreshMainScreen();
          DrawTextBox(40, 0, 0, true, red("Deletion Cancelled >:D"));
          UpdateStatus(red("The Book Survived!"));
          cin.get();
          return;
      }


        }
      }
    }

      if (!found) {
        DrawTextBox(30, 0, 0, true, red("ID NOT FOUND"));
        UpdateStatus(red("Book not found. yey?"));
        cin.get();
        return;
      }

}

string searched = ""; //for Highlight hehe
bool isSearching = false;
void SearchBook(vector<Book>& Library) {
    RefreshUI(SEARCH);
    PrintState(SEARCH);
    PrintLegends(SEARCH);

if (Library.size() == 0) {
  DrawTextBox(50, 0, 0, true, red("NO BOOKS TO SEARCH"));
  UpdateStatus(red("Tumbleweed..."));
  cin.get();
  return;
}

    vector<Book> results;
    string input;

    DrawTextBox(50, 0, 0, true, cyan("Enter Keyword"));
    UpdateStatus(cyan("Searching... sEaRcHiNg... sertsing..."));
    ResetCursorToInput();
    getline(cin, input);

    string loweredInput = ToLowerWord(input);
    searched = loweredInput; // reminder this is a global var
    isSearching = true; // testing

    results.clear();
    // global searching mueheheehe
    for (const Book& b : Library) {
      // Check every field against the same input
      if (ToLowerWord(b.title).find(loweredInput)    != string::npos ||
          ToLowerWord(b.author).find(loweredInput)   != string::npos ||
          ToLowerWord(b.genre).find(loweredInput)    != string::npos ||
          ToLowerWord(b.category).find(loweredInput) != string::npos ||
          to_string(b.ID) == input) { // Exact match for ID

          results.push_back(b);
      }
    }

    if (input.empty()) {
    UpdateStatus(red("No input... went back to " + cyan("'View Mode'")));
    return;
    }

    // Show results
    if (results.empty()) {
		DrawTextBox(50, 0, 0, true, red("BOOK NOT FOUND"));
        UpdateStatus(red("No books/author found. Going back..."));
        cin.get();
        return;
    }

    UpdateStatus(cyan("Showing results for: " + bold(purple(input))));
    ViewAllBook(results, SEARCH);
}



void BorrowBook(vector<Book>& Library) {
RefreshUI(BORROW);
PrintState(BORROW);
PrintLegends(BORROW);
UpdateStatus(cyan("Borrowing a book..."));
if (Library.size() == 0) {
  DrawTextBox(50, 0, 0, true, red("NO BOOKS TO BORROW"));
  UpdateStatus(red("Aww no books :("));
  cin.get();
  return;
}
showHint();

auto warning = []() {
  int startX = consoleWidth/2;
  int startY = consoleHeight/2;
  vector<string> text = {
  "Borrowed books must be returned on or before the due date.",
  "Failure to return by the due date will result in penalties.",
  "Estimated fine per day late: " + yellow("₱5.00") + bg(),
  };

  RefreshMainScreen();
  DrawTextBox(30, 0, -4, true, (yellow("IMPORTANT:")));
  for (int i = 0; i < text.size(); i++) {
  SetCursorIn(startX-(stripAnsi(text[i]).length()/2), startY+i);
  if (i == 2) {SetCursorIn(startX-(stripAnsi(text[i]).length()/2), startY+i+1);}
  cout << bg() << txtcol() << text[i] << string(consoleWidth-3 - (stripAnsi(text[i]).length()/2)-(consoleWidth/2), ' ');
  }
  UpdateStatus(green("Press any key to acknowledge..."));
  cin.get();
  RefreshMainScreen();
};


  DrawTextBox(40, 0, 0, true, cyan("Enter Book ID"));
  ResetCursorToInput();

  string tempId;
  getline(cin, tempId);

  if (tempId.empty()) {
    UpdateStatus(red("No input, going backwards..."));
    cin.get();
    return;
  }

  int ID;
  if (isNumber(tempId)) {
    ID = stoi(tempId);
  } else {
    DrawTextBox(40, 0, 0, true, red("INVALID ID"));
    UpdateStatus(red("Invalid ID boss"));
    cin.get();
    return;
  }

  bool found = false;

  for (Book& b : Library) {
    if (ID == b.ID) {
      found = true;

      while (true) {
      RefreshUI(BORROW);

      // if available edi nandyan
      if (!b.isAvailable) {
      DrawTextBox(40, 0, 0, true, red("BOOK BORROWED"));
      ResetCursorToInput();
      UpdateStatus(red("Awww... Someone took it :P"));
      cin.get();
      return;
      }

      warning();
      PrintTableHeaders();
      SetCursorIn(3, 7);
      PrintBookDetails(0, b);
      UpdateStatus(green("Hell Yeah! I HAVE that Book here..."));
      DrawTextBox(30, 0, 0, true, yellow("Borrow Book? (y/n)"));
      ResetCursorToInput();

      char choice = tolower(cin.get());
      cin.ignore(1000, '\n');

      if (choice == 'y') {
      DrawTextBox(30, 0, 0, true, cyan("Enter your NAME"));
      UpdateStatus(green("May I know the name of the reader..?"));

      string name;
      getline(cin, name);
      if (name.empty()) {
        UpdateStatus(red("Invalid Name Try Again..."));
        cin.get();
        continue;
      }

      DrawTextBox(30, 0, 0, true, green("HAVE FUN :)"));
      DrawTextBox(26, 0, 5, true, yellow("Due Date: ") + bg() + txtcol() + bold(GetDate(setDueDate)));
      UpdateStatus(green("Bye bye book... GoodLuck..."));
      b.isAvailable = false;
      b.dateBorrowed = GetDate(0);
      b.dueDate = GetDate(setDueDate);
      b.borrowerName = name;
      Log("Book ID " + to_string(b.ID) + " has been borrowed by '" + name + "'");
      Log("Book ID " + to_string(b.ID) + " Due Date set to '" + GetDate(setDueDate) + "'");
      cin.get();
      return;
      }

      if (choice == 'n') {
      DrawTextBox(50, 0, 0, true, cyan("BORROWING CANCELLED"));
      UpdateStatus(cyan("Read some books next time..."));
      cin.get();
      return;
      }

    }
    }
  }

  if (!found) {
    DrawTextBox(40, 0, 0, true, red("BOOK NOT FOUND"));
    UpdateStatus(red("Maybe it got deleted idk"));
    cin.get();
    return;
  }

}




void ReturnBook(vector<Book>& Library) {
RefreshUI(RETURN);
PrintState(RETURN);
PrintLegends(RETURN);
UpdateStatus(cyan("Returning a book..."));
if (Library.size() == 0) {
  DrawTextBox(50, 0, 0, true, red("NO BOOKS TO RETURN"));
  UpdateStatus(red("We just opened, that ain't ours..."));
  cin.get();
  return;
}
auto warning = [](Book& b) {
  int startX = consoleWidth/2;
  int startY = consoleHeight/2;
  vector<string> text = {
   "This book is " + red("overdue") + bg() + txtcol() + " and has exceeded its due date.",
   ("Returning it now will incur a fine of " + yellow("P5.00") + bg() + txtcol() + " per day late."),
   "Please proceed only if you acknowledge the penalty.",
   "Total accumulated fine penatly: " + yellow("₱"+to_string(GetFine(b.dueDate))+".00") + bg(),
  };

  RefreshMainScreen();
  DrawTextBox(30, 0, -4, true, (red("NOTICE:")));
  for (int i = 0; i < text.size(); i++) {
  SetCursorIn(startX-(stripAnsi(text[i]).length()/2), startY+i);
  if (i == 3) {SetCursorIn(startX-(stripAnsi(text[i]).length()/2), startY+i+1);}
  cout << bg() << txtcol() << text[i] << string(consoleWidth-3 - (stripAnsi(text[i]).length()/2)-(consoleWidth/2), ' ');
  }
  UpdateStatus(yellow("Press any key to acknowledge..."));
  cin.get();
  cin.ignore();
  RefreshMainScreen();
};

  DrawTextBox(40, 0, 0, true, cyan("Enter Book ID"));
  ResetCursorToInput();

  string tempId;
  getline(cin, tempId);

  if (tempId.empty()) {
    UpdateStatus(red("No input, going backwards..."));
    cin.get();
    return;
  }

  int ID;
  if (isNumber(tempId)) {
    ID = stoi(tempId);
  } else {
    DrawTextBox(40, 0, 0, true, red("INVALID ID"));
    UpdateStatus(red("Invalid ID boss"));
    cin.get();
    return;
  }

  bool found = false;

  for (Book& b : Library) {
    if (ID == b.ID) {
      found = true;

      while (true) {
      RefreshUI(RETURN);
      if (GetDate(today) > b.dueDate && !b.isAvailable) { warning(b); }

      PrintTableHeaders();
      SetCursorIn(3, 7);
      PrintBookDetails(0, b);

      // if available edi nandyan
      if (b.isAvailable) {
      DrawTextBox(40, 0, 0, true, red("BOOK ALREADY HERE"));
      ResetCursorToInput();
      UpdateStatus(red("That book seems to be available here"));
      cin.get();
      return;
      }

      UpdateStatus(green("Yeh... that book belongs here..."));
      DrawTextBox(40, 0, 0, true, yellow("Return Book? (y/n)"));
      ResetCursorToInput();

      char choice = tolower(cin.get());
      cin.ignore(1000, '\n');

      if (choice == 'y') {
      DrawTextBox(50, 0, 0, true, green("Thank You for Returning! :D"));
      UpdateStatus(green("Book has RETURNED!"));
      b.isAvailable = true;
      Log("Book ID " + to_string(b.ID) + " has been returned");
      cin.get();
      return;
      }

      if (choice == 'n') {
      DrawTextBox(50, 0, 0, true, red("RETURN CANCELLED"));
      UpdateStatus(cyan("Make sure to return that someday!"));
      cin.get();
      return;
      }

    }
    }
  }

  if (!found) {
    DrawTextBox(40, 0, 0, true, red("BOOK NOT FOUND"));
    UpdateStatus(red("There is NO such book here."));
    cin.get();
    return;
  }

}





void ViewAllBook(vector<Book>& Library, MENU mode) {
RefreshUI(VIEWALL);
PrintState(VIEWALL);
PrintLegends(VIEWALL);
int startIndex = 0; // start page (by rows)
int startY = 7; // the start of the main display
bool isAscending = false;

if (Library.size() == 0) {
  DrawTextBox(40, 0, 0, true, red("EMPTY LIBRARY"));
  UpdateStatus(red("Nothing to see here brochacho...."));
  cin.get();
  return;
}

while (true) {
  PrintState(VIEWALL);
  PrintLegends(VIEWALL);
    // this if for resized, reset all to beginning
    getScreenSize(consoleWidth, consoleHeight); // get the prev size
    if (consoleWidth != prevW || consoleHeight != prevH) {
        RefreshUI(VIEWALL);
        PrintLegends(VIEWALL);
    }
    int maxHeight = consoleHeight - 8; // excluding footer and header
    int pageSize = maxHeight - startY;
    int totalBooks = Library.size(); // how many books in our library
    int totalPages = (totalBooks + pageSize - 1) / pageSize;
    int currentPage = (startIndex / pageSize) + 1;

  // validate first
  if (startIndex < 0) { startIndex = 0; currentPage = 1; } // dont go negative
  if (totalBooks > 0 && startIndex >= totalBooks) {
      startIndex = ((totalBooks - 1) / pageSize) * pageSize;
      currentPage--;
  }


  // then print :D
  ListAllBooks(Library, startIndex);
  PrintPage(currentPage, totalPages);

  char input = tolower(cin.get());
  if (input == '\n') { continue; }
  cin.ignore(1000, '\n');

  if ('v' == input) { ShowFullBookDetails(Library); UpdateStatus(cyan("Exited Book..."));}

  if ('s' == input) {
    if (mode == SEARCH) {
        UpdateStatus(red("Already in search mode! Press 'x' to go back first."));
    } else {
      SearchBook(Library);
      isSearching = false;
      UpdateStatus(cyan("Exited Search Mode"));
    }
  }

    if ('f' == input) {
    if (mode == FILTER) {
        UpdateStatus(red("Already in filter mode! Press 'x' to go back first."));
    } else {
      FilterBooks(Library);
      isSearching = false;
      UpdateStatus(cyan("Exited Filter Mode"));
    }
  }

  switch (input) {
    case '1': //Title
    isAscending = !isAscending;
    sort(Library.begin(), Library.end(), [&](const Book& a, const Book& b) {
      return (isAscending) ? ToLowerWord(a.title) < ToLowerWord(b.title) :
                             ToLowerWord(a.title) > ToLowerWord(b.title);
    });  UpdateStatus(cyan("Sorted by: ") + bg() + yellow("TITLE ")
                      + bg() + ((isAscending) ? green("▲") : green("▼")));
    startIndex = 0;
    break;

    case '2': //Author
    isAscending = !isAscending;
    sort(Library.begin(), Library.end(), [&](const Book& a, const Book& b) {
      return (isAscending) ? ToLowerWord(a.author) < ToLowerWord(b.author) :
                             ToLowerWord(a.author) > ToLowerWord(b.author);
    });  UpdateStatus(cyan("Sorted by: ") + bg() + yellow("AUTHOR ")
                      + bg() + ((isAscending) ? green("▲") : green("▼")));
    startIndex = 0;
    break;

    case '3': //Year latest to earliest
    isAscending = !isAscending;
    sort(Library.begin(), Library.end(), [&](const Book& a, const Book& b) {
      return (isAscending) ? a.year > b.year : a.year < b.year;
    }); UpdateStatus(cyan("Sorted by: ") + bg() + yellow("YEAR ")
                    + bg() + ((isAscending) ? green("▲") : green("▼")));
    startIndex = 0;
    break;

    case '4': //Category
    isAscending = !isAscending;
    sort(Library.begin(), Library.end(), [&](const Book& a, const Book& b) {
      return (isAscending) ? a.category > b.category : a.category < b.category;
    });  UpdateStatus(cyan("Sorted by: ") + bg() + yellow("CATEGORY ")
                      + bg() + ((isAscending) ? green("▲") : green("▼")));
    startIndex = 0;
    break;

    case '5': //Genre
    isAscending = !isAscending;
    sort(Library.begin(), Library.end(), [&](const Book& a, const Book& b) {
      return (isAscending) ? a.genre > b.genre : a.genre < b.genre;
    });  UpdateStatus(cyan("Sorted by: ") + bg() + yellow("GENRE ")
                      + bg() + ((isAscending) ? green("▲") : green("▼")));
    startIndex = 0;
    break;

    case '6': //Status
    isAscending = !isAscending;
    sort(Library.begin(), Library.end(), [&](const Book& a, const Book& b) {
      return (isAscending) ? a.isAvailable > b.isAvailable : a.isAvailable < b.isAvailable;
    });  UpdateStatus(cyan("Sorted by: ") + bg() + yellow("AVAILABILITY ")
                     + bg() + ((isAscending) ? green("▲") : green("▼")));
    startIndex = 0;
    break;

    case '7': //ID
    isAscending = !isAscending;
    sort(Library.begin(), Library.end(), [&](const Book& a, const Book& b) {
      return (isAscending) ? a.ID < b.ID : a.ID > b.ID;
    }); UpdateStatus(cyan("Sorted by: ") + bg() + yellow("ID ")
                     + bg() + ((isAscending) ? green("▲") : green("▼")));
    startIndex = 0;
    break;
  }

  if (input == 'n') { startIndex += pageSize; } // next page
  if (input == 'b') { startIndex -= pageSize; } // back
  if (input == 'x') { break; }

}
}


void Exit() {
UpdateStatus(cyan("Exiting the program..."));
DrawTextBox(50, 0, 0, true, red("Do you really wanna exit? (y/N)"));
ResetCursorToInput();

if (cin.get() != 'y') {
  UpdateStatus(green("Cancelled exit."));
  DrawTextBox(50, 0, 0, true, green("Yey, you didn't leave :D"));
  ResetCursorToInput();
  cin.get();
} else {
  DrawTextBox(50, 0, 0, true, cyan("GoodBye World :("));
  ResetCursorToInput();
  cin.get();
  cin.ignore();
  Log("Exited the program...");
  Clear();
  exit(0);
}
}


//===========================================================================================================
//Helper Functions goes here
//note: if you gon add nother one, forward declare it on the top


int GetID(vector<Book>& lib) {
    int maxID = 0;
    for (const Book& b : lib) {
        if (b.ID > maxID) { maxID = b.ID; }
    }
    return maxID + 1;
}



void ListAllBooks(vector<Book>& Library, int startIndex) {
    PrintTableHeaders(); // desc
    int maxHeight = consoleHeight - 8;
    int startX = 3;
    int startY = 7;
    int visibleRows = maxHeight - startY;
    int totalBooks = Library.size();

    int i = 0;

    // print books
    for (; i < visibleRows && (startIndex + i) < totalBooks; i++) {
        SetCursorIn(startX, startY + i);
        PrintBookDetails(0, Library[startIndex + i]);
    }

    // check if there are MORE books after this page
    bool hasNextPage = (startIndex + i) < totalBooks;

    if (hasNextPage) {
        SetCursorIn(startX, startY + i);
        PrintBookDetails(1, Library[0]); // "..." row
        i++;
    }

    // clear leftover rows
    for (; i < visibleRows+1; i++) {
        SetCursorIn(startX, startY + i);
        cout << string(consoleWidth - 3, ' ');
    }

    ResetCursorToInput();
}


void ListGenre(string pickedCategory) {
  Genre g;
  RefreshMainScreen();
  int startX = 3; // Starting X position for the genre list
  int startY = 5; // Starting Y position for the genre list
  int maxHeight = (consoleHeight - 8) + startY;

  if (pickedCategory == "1") {
  SetCursorIn(startX, startY);
  cout << txtcol() << bold("Fiction Genre:");
  for (int i = 0; i < g.Fiction.size(); i++) {
    SetCursorIn(startX, startY + i + 1);
    cout << txtcol() << bg() << "( " << bold(yellow(to_string(i+1))) << bg() << " ) " << txtcol() << g.Fiction[i] << "\n";
  }
}

if (pickedCategory == "2") {
  SetCursorIn(startX, startY);
  cout << txtcol() << bold("Non-Fiction Genre:");
  for (int i = 0; i < g.nonFiction.size(); i++) {
    SetCursorIn(startX, startY + i + 1);
    cout << txtcol() << bg() << "( " << yellow(to_string(i+1)) << bg() << " ) " << txtcol()<< g.nonFiction[i] << "\n";
  }
}
}

void FilterBooks(vector<Book>& Library) {
  RefreshUI(FILTER);
  PrintLegends(FILTER);
  PrintState(FILTER);
  isSearching = true;

  auto filterYear = []() {
    string input;

    while (true) {
    RefreshUI(FILTER);
    DrawTextBox(50, 0, 0, true, cyan("Enter Year"));
    UpdateStatus(cyan("Filtering Year"));    getline(cin, input);
    if (!isYearValid(input)) {
      UpdateStatus(red("Invalid year"));
      continue;
    }
      break;
  }
    return input;
  };

  auto filterCategory = [&]() {
    string input;
    string pickedCategory;
    while (true) {
      RefreshUI(FILTER);

      DrawTextBox(50, 0, 0, true, cyan("Choose Genre"));
      DrawTextBox(50, 0, 5, true, yellow("1") + bg() + cyan(" for Fiction | ")
                  + bg() + bold(yellow("2")) + bg() + cyan(" for Non-Fiction"));
      UpdateStatus(cyan("Filtering Category"));
      getline(cin, input);
      if (input.empty()) { break; }
      if (!isCategoryValid(input, pickedCategory)) {
      UpdateStatus(red("Invalid Category"));
      continue;
      }
      break;
    }
    return pickedCategory;
  };

    auto filterGenre = [&]() {
    string input;
    string pickedCategory;
    string pickedGenre;
    string picked;

    while (true) {
      RefreshUI(FILTER);

      DrawTextBox(50, 0, 0, true, cyan("Choose Category"));
      DrawTextBox(50, 0, 5, true, yellow("1") + bg() + cyan(" for Fiction | ")
                + bg() + bold(yellow("2")) + bg() + cyan(" for Non-Fiction"));
      UpdateStatus(cyan("Filtering Genre"));
      getline(cin, input);
      if (input.empty()) { break; }
      if (!isCategoryValid(input, pickedCategory)) {
      UpdateStatus(red("Invalid Category"));
      continue;
      }
      RefreshMainScreen();
      ListGenre(input);
      UpdateStatus(cyan("Choosing Genre"));
      getline(cin, picked);
      if (!isGenreValid(picked, pickedCategory, pickedGenre)) {
      UpdateStatus(red("Invalid Genre"));
      continue;
      }
      break;
    }
    return pickedGenre;
  };


  DrawTextBox(50, 0, 0, true, cyan("Filter By?"));
  UpdateStatus(cyan("Filtering..."));

  string input;
  getline(cin, input);

  if(input.empty()) {
    UpdateStatus(red("No input... Cancelled!"));
    return;
  }

  int choice;
  if (isNumber(input)) {
    choice = stoi(input);
  } else {
    UpdateStatus(red("Invalid input, choose from the options..."));
    return;
  }

  string selected;
  switch (choice) {
    case 1: selected = filterYear();
    searched = selected;
    break;
    case 2: selected = filterCategory();
    searched = selected;
    break;
    case 3: selected = filterGenre();
    searched = selected;
    break;

    case 4: selected = "1"; break;
    case 5: selected = "0"; break;
    case 6: selected = GetDate(today); break;

    default:
    UpdateStatus(red("Choose from the options bruh."));
    cin.get(); return;
    break;
  }

  if (selected.empty()) {
    UpdateStatus(red("No input. going back"));
    cin.get();
    return;
  }
  vector<Book> results;
  for (Book& b : Library) {
    switch (choice) {
      case 1: if (b.year == stoi(selected)) { results.push_back(b); } break;
      case 2: if (b.category == selected) { results.push_back(b); } break;
      case 3: if (b.genre == selected) { results.push_back(b); } break;
      case 4: if (b.isAvailable) { results.push_back(b); } break;
      case 5: if (!b.isAvailable && GetDate(today) < b.dueDate) { results.push_back(b); } break;
      case 6: if (selected > b.dueDate && !b.isAvailable) { results.push_back(b); } break;
    }
  }

    switch (choice) {
      case 1: UpdateStatus(cyan("Filtered by Year: " + purple(selected) ));     break;
      case 2: UpdateStatus(cyan("Filtered by Category: " + purple(selected) )); break;
      case 3: UpdateStatus(cyan("Filtered by Genre: " + purple(selected) ));    break;
      case 4: UpdateStatus(cyan("Filtered by Status: " + green("AVAILABLE") )); break;
      case 5: UpdateStatus(cyan("Filtered by Status: " + red("BORROWED") ));    break;
      case 6: UpdateStatus(cyan("Filtered by Status: " + bold(red("OVERDUE"))));    break;
    }

  if (results.empty()) {
  UpdateStatus(red("No books found matching your criteria."));
  cin.get();
  return;
  }

  ViewAllBook(results, FILTER);
}

void ShowFullBookDetails(vector<Book>& Library) {
  DrawTextBox(30, 0, 0, true, cyan("Enter ID to VIEW"));
  ResetCursorToInput();

    bool found = false;
    string tempId;
    getline(cin, tempId);
    if (tempId.empty()) {
      UpdateStatus(red("No input"));
      return;
    }

    int ID;
    if (isNumber(tempId)) {
      ID = stoi(tempId);
    } else {
      DrawTextBox(50, 0, 0, true, red("INVALID BOOK ID"));
      UpdateStatus(red("No matching ID found"));
      cin.get();
      return;
    }

    for (Book& b : Library) {
      if (ID == b.ID) {
      found = true;
      RefreshMainScreen();
      SetCursorIn(3, 5);
      cout << orange("Book Full Details:");
      SetCursorIn(3, 6);  cout << left <<bg()<< yellow("Book ID: ")        <<setw(43)<<bg()<<txtcol() << b.ID;
      SetCursorIn(3, 7);  cout << left <<bg()<< yellow("Title: ")          <<setw(45)<<bg()<<txtcol() << b.title;
      SetCursorIn(3, 8);  cout << left <<bg()<< yellow("Author: ")         <<setw(44)<<bg()<<txtcol() << b.author;
      SetCursorIn(3, 9);  cout << left <<bg()<< yellow("Year Published: ") <<setw(36)<<bg()<<txtcol() << b.year;
      SetCursorIn(3, 10); cout << left <<bg()<< yellow("Category: ")       <<setw(42)<<bg()<<txtcol() << b.category;
      SetCursorIn(3, 11); cout << left <<bg()<< yellow("Genre: ")          <<setw(45)<<bg()<<txtcol() << b.genre;
      SetCursorIn(3, 12); cout << left <<bg()<< yellow("Date Added: ")     <<setw(40)<<bg()<<txtcol() << b.dateAdded;
      SetCursorIn(3, 14); cout << left <<bg()<< yellow("Status: ")         <<setw(44)<<bg()<<txtcol() << ((b.isAvailable) ? green("Available") : red("Borrowed"));

      if (!b.isAvailable) {
      SetCursorIn(3, 15); cout << left <<bg()<< yellow("Borrower Name: ")  <<setw(37)<<bg()<<txtcol() << b.borrowerName;
      SetCursorIn(3, 16); cout << left <<bg()<< yellow("Date Borrowed: ")  <<setw(37)<<bg()<<txtcol() << b.dateBorrowed;
      SetCursorIn(3, 17); cout << left <<bg()<< yellow("Due Date: ")       <<setw(42)<<bg()<<txtcol() << b.dueDate << bg() <<
                          ((GetDate(today) > b.dueDate) ? " ( "  + bold(red("OVERDUE")) + bg() + " )" : "");
      }
      if (GetDate(today) > b.dueDate && !b.isAvailable) {
      int fine = GetFine(b.dueDate);
      SetCursorIn(3, 18); cout << left <<bg()<< yellow("Total Fine: ")  <<setw(40)<<bg()<<txtcol() << "₱" << fine << ".00";
      }
    }
    }

    if (!found) { UpdateStatus(red("No matching ID...")); cin.get(); return;}
    ResetCursorToInput();
    UpdateStatus(cyan("Showing Book Full Details..."));
    cin.get();
}

void Clear() {
  // some key codes to clear screen
  cout << "\033[2J\033[3J\033[H";
}

string ToLowerWord(string text) {
// hehe small function for temporary lowercase
// needed for sorting cuz sort() doesnt mix lowercased and uppercased
    transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}

void SaveFile(vector<Book>& Library) {
  Book b;
  ofstream outFile("BookRecords.csv");

    outFile << "sep=|\n";
    outFile << "Title|Author|Year|Category|Genre|ID|Status|Date Added|Borrower Name|Date Borrowed|Due Date\n";

  if (outFile.is_open()) {
  // for every added inputs(books) in the array(library) saves it
    for (const Book& b : Library) {
  //  file    |     data      |  separator
      outFile <<  b.title         << '|'
              <<  b.author        << '|'
              <<  b.year          << '|'
              <<  b.category      << '|'
              <<  b.genre         << '|'
              <<  b.ID            << '|'
              <<  b.isAvailable   << "|"
              <<  b.dateAdded     << "|"
              <<  b.borrowerName  << "|"
              <<  b.dateBorrowed  << "|"
              <<  b.dueDate       << "\n";

    }
    outFile.close();
  }
}

void LoadFile(vector<Book>& Library) {
  Book b;
  Library.clear();

    ifstream inFile("BookRecords.csv");
    if(!inFile){
        string msg = "No File Found... Starting Fresh...";
        cout << red(msg);
        cin.get();
        return;
    }


  string line;
  string tempYear;
  string tempID;
  string tempStatus;

    if (getline(inFile, line)) { /* skip This is for sep=| */ }
    if (getline(inFile, line)) { /* skip This is for the headers for excel */ }

    while (getline(inFile, line)) {
      stringstream ss(line);
        if (line.empty()) continue; //skip any empty lines in the file

  //    getter  | line |  data/input | delimiter/stopper
        getline(  ss,     b.title,          '|');
        getline(  ss,     b.author,         '|');
        getline(  ss,     tempYear,         '|');
        getline(  ss,     b.category,       '|');
        getline(  ss,     b.genre,          '|');
        getline(  ss,     tempID,           '|');
		    getline(  ss,     tempStatus,       '|');
		    getline(  ss,     b.dateAdded,      '|');
		    getline(  ss,     b.borrowerName,   '|');
		    getline(  ss,     b.dateBorrowed,   '|');
		    getline(  ss,     b.dueDate,        '|');


        if (!tempYear.empty()) {
            b.year = stoi(tempYear);
        } //convert that to int, cuz getline cant read ints

        if (!tempID.empty()) {
            b.ID = stoi(tempID);
        } //convert that to int, cuz getline cant read ints

		if (!tempStatus.empty()) {
		 	b.isAvailable = (tempStatus == "1");
		}

        Library.push_back(b); // Send all the data into the array
     }

   inFile.close();
}

void Log(string action) {
    ofstream log("logs.txt", ios::app);
    log << "[" << GetDate(0) << " " << GetTime() << "] " << action << "\n";
}

string GetDate(int daysFromNow) {
  auto AddDays = [](time_t base, int days) {
    return base + (days * 24 * 60 * 60);
};
    time_t now = time(0);
    time_t due = AddDays(now, daysFromNow);

    tm* local = localtime(&due);

    int y = local->tm_year + 1900;
    int m = local->tm_mon + 1;
    int d = local->tm_mday;

    string yyyy = to_string(y);
    string mm = (m < 10 ? "0" : "") + to_string(m);
    string dd = (d < 10 ? "0" : "") + to_string(d);

    return yyyy + "-" + mm + "-" + dd;
}

string GetTime() {
    time_t now = time(0);
    tm* local = localtime(&now);

    int h = local->tm_hour;
    int m = local->tm_min;
    int s = local->tm_sec;

    return (h < 10 ? "0" : "") + to_string(h) + ":" +
           (m < 10 ? "0" : "") + to_string(m) + ":" +
           (s < 10 ? "0" : "") + to_string(s);
}

int GetFine(string setDueDate) {
    time_t dueDate = ParseDate(setDueDate);
    time_t now = (TEST_DATE) ? ParseDate(GetDate(today)) : time(0);
    int finePerDay = 5; // 5 petot

    double secondsLate = difftime(now, dueDate);

    if (secondsLate <= 0) { return 0; }

    int overdueDays = secondsLate / (24 * 60 * 60);

    return overdueDays * finePerDay;
}

time_t ParseDate(string date) {
    tm t = {};

    t.tm_year = stoi(date.substr(0,4)) - 1900;
    t.tm_mon  = stoi(date.substr(5,2)) - 1;
    t.tm_mday = stoi(date.substr(8,2));

    return mktime(&t);
}

//==========================================================================================================
//Validator Functions goes here


bool isNumber(const string& s) {
    if (s.empty()) return false; // if theres none edi wala
    return all_of(s.begin(), s.end(), ::isdigit); // if all is a digit edi goods
}

bool isYearValid(string tempyear) {
      if (tempyear.empty()) { return false;}

      if (!isNumber(tempyear)) {
        UpdateStatus(red("Please input numbers..."));
        return false;
      }


      int year = stoi(tempyear);
      if (year > 2026 || year < 0) {
            UpdateStatus(red("Invalid input! Please enter a valid year."));
            return false;
        }

        if (year < 1000) {
        UpdateStatus(green("Yes, We do accept scriptures from Year '") + bg() + yellow(tempyear) + bg() + green("'"));
        } else {
        UpdateStatus(green("Book's Year is set to '") + bg() + yellow(tempyear) + bg() + green("'"));
		}
        return true;
}

bool isCategoryValid(string& input, string& output) {
      if (input != "1" && input != "2") {
        UpdateStatus(red("Please input either: ") + "( " + yellow("1") + " ) Fiction, ( " + yellow("2") + " ) Non-Fiction.");
        return false;
      }

      output = (input == "1") ? "Fiction" : "Non-Fiction";
      UpdateStatus(green("Book's Category is set to '") + bg() + yellow(output) + bg() + green("'"));
      return true;
}

bool isGenreValid(string tempgenre, string& pickedCategory, string& outGenre) {

      Genre g;
      if (!isNumber(tempgenre)) {
        UpdateStatus(red("Please input numbers..."));
        return false;
      }


      int size;
      if (pickedCategory == "Fiction") { size = g.Fiction.size(); }
      if (pickedCategory == "Non-Fiction")  { size = g.nonFiction.size(); }

      int genreChoice = stoi(tempgenre);

      if (genreChoice < 1 || genreChoice > size) {
            UpdateStatus(red("Please choose one from the list: 1 - " + to_string(size)));
            return false;
        }

      if (pickedCategory == "Fiction") { outGenre = g.Fiction[genreChoice-1]; }
      if (pickedCategory == "Non-Fiction")  { outGenre = g.nonFiction[genreChoice-1]; }

        UpdateStatus(green("Book's Genre is set to '") + bg() + yellow(outGenre) + bg() + green("'"));
        return true;
}







//===========================================================================================================
//UI helpers goes here



//coloring thingies, just pass the text and it will return the colored text
string cyan(string t)   { return "\033[38;2;131;165;152m" + t + "\033[0m"; } // #83a598 (Bright Blue)
string yellow(string t) { return "\033[38;2;250;189;47m" + t + "\033[0m"; }  // #fabd2f (Bright Yellow)
string green(string t)  { return "\033[38;2;184;187;38m" + t + "\033[0m"; }  // #b8bb26 (Bright Green)
string red(string t)    { return "\033[38;2;251;73;52m" + t + "\033[0m"; }   // #fb4934 (Bright Red)
string orange(string t) { return "\033[38;2;254;128;25m" + t + "\033[0m"; }  // #fe8019 (Bright Orange)
string purple(string t) { return "\033[38;2;211;134;155m" + t + "\033[0m"; } // #d3869b (Bright Purple)
string bold(string t)   { return "\033[1m" + t + "\033[0m"; }



void RefreshUI(MENU m) {
  getScreenSize(consoleWidth, consoleHeight);
  //only redraw if theres a change in screensize
  if (consoleWidth != prevW || consoleHeight != prevH) {
      Clear();
      DrawUI();
      PrintState(m);
      PrintLegends(m);
      UpdateStatus(cyan("Screen Resized!"));
      prevW = consoleWidth;
      prevH = consoleHeight;

      if (consoleWidth < 90 || consoleHeight < 20) {
      	UpdateStatus(red("Screen kinda too smol... UI may break... :P"));
      }
    if (consoleWidth < 80 || consoleHeight < 15) {
    Clear();
    // cout << red("Screen too small. Please resize terminal.\n");
    DrawTextBox(50, 0, 0, true, red("Screen too small. Please resize terminal."));
    return;
}
}
}

//OS based screen size getter, for the fullscreen implementation
void getScreenSize(int &cols, int &rows) {
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            // 1. Force UTF-8 encoding for fancy box-drawing characters
    SetConsoleOutputCP(CP_UTF8);

    // 2. Enable ANSI Escape Codes (for colors and cursor jumps)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    #else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        cols = w.ws_col;
        rows = w.ws_row;
    #endif
}

void DrawUI() {
  while (true) {
    getScreenSize(consoleWidth, consoleHeight);
    if (consoleWidth < 80 || consoleHeight < 15) {
    Clear();
    // cout << red("Screen too small. Please resize terminal.\n");
    DrawTextBox(50, 0, 0, true, red("Screen too small. Please resize terminal."));
    cin.get();
    continue;
    }
    break;
  }
  DrawHeader();
  DrawMainScreen();
  DrawFooter();
}

void DrawMainScreen() {
int maxHeight = consoleHeight - 12; // 12 lines reserved for header and footer (6 for header, 6 for footer)
    PrintLine("╔", "═", "╗"); // top border
     for (int i = 0; i < maxHeight; ++i) {
      SetCursorIn(1, 5 + i);
      PrintLine("║", " ", "║"); // sides with space in between
     }
    PrintLine("╚", "═", "╝"); // bottom border
}

void RefreshMainScreen() {
  int maxHeight = consoleHeight - 12; // 14 lines reserved for main screen borders, header and footer (2 for borders, 6 for header, 6 for footer)
  for (int i = 0; i < maxHeight; ++i) {
    SetCursorIn(2, 5 + i); // move cursor to the beginning of each line
    PrintLine("", " ", "");
  }
}

void DrawHeader() {
    string title = "LIBRARY OF ALEXANDRIA";
    SetCursorIn(1, 1);
    cout << bg() << legcol() << string(consoleWidth, '=');
    SetCursorIn(1, 2);
    cout << bg() << legcol() << string(consoleWidth/2 - title.length()/2, ' ') << bold(gold_line(title)) << bg() << string(consoleWidth/2 - title.length()/2, ' ');
    SetCursorIn(1, 3);
    cout << bg() << legcol() << string(consoleWidth, '=');
}

void DrawFooter() {

    int startRow = consoleHeight - 6;
    cout << "\033[" << startRow << ";1H";
    MENU m;
    PrintLine("┌", "─", "┐");
    // legends/choices
    PrintLine("│", " ", "│"); // empty line for spacing
    PrintLegends(HOME);
    PrintLine("├", "─", "┤");
    PrintLine("│", " ", "│"); // empty line for spacing
    PrintLine("├", "─", "┤");
    PrintLine("│", " ", "│"); // empty line for spacing
    PrintLine("└", "─", "┘");

    ResetCursorToInput();
}


void UpdateStatus(string status) {
    string prompt = "STATUS: ";

    string visible = stripAnsi(status);

    if (visible.length() > consoleWidth-18) {
      status = fitTextWidth(visible, consoleWidth-18);
    }
    // delete the previous thingies inputs
    cout << bg() << "\33[" << consoleHeight - 1 << ";3H" << string(consoleWidth - (3+25), ' ');
    // Move cursor to the status line
    cout << bg() << "\033[" << consoleHeight - 1 << ";" << prompt.length() << "H"; // 11 is the column where the status starts
    cout << bg() << "\33[" << consoleHeight - 1 << ";3H" << legcol() << prompt;
    cout << bg() << status << bg() << string((consoleWidth-(3+25)) - visible.length(), ' '); // Print status and clear remaining space


    // then print cursor again
    ResetCursorToInput();
}

void DrawTextBox(int boxWidth, int x, int y, bool isColored, string text) {

    // the coordinates x and y will start at the center of the console
    // for easy pointing

    int boxHeight = 3; // 1 line for text, 2 lines for borders

    int midBoxHeight = boxHeight/2;
    int midBoxWidth = boxWidth/2;
    int anchorPointX = (consoleWidth / 2) + x - midBoxWidth;
    int anchorPointY = (consoleHeight / 2) + y - midBoxHeight;

    // this puts the cursor to the top left corner of the box
    // but the real anchor point is the center of the box
    cout << "\033[" << anchorPointY - 1 << ";" << anchorPointX  << "H";

    cout << bg() << legcol() << "╭";
    for(int i = 0; i < boxWidth - 2; ++i) cout << "─";
    cout << bg() << legcol() << "╮";

    for (int i = 0; i < boxHeight; ++i) {
        cout << "\033[" << anchorPointY + i << ";" << anchorPointX << "H";
        cout << bg() << legcol() << "│" << string(boxWidth - 2, ' ') << "│\n";
    }

    cout << "\033[" << anchorPointY + boxHeight << ";" << anchorPointX << "H";
    cout << bg() << legcol() << "╰";
    for(int i = 0; i < boxWidth - 2; ++i) cout << "─";
    cout << bg() << legcol() << "╯";

    cout << "\033[" << anchorPointY + midBoxHeight << ";" << anchorPointX+1 << "H";

    if (isColored) {
    string clean = stripAnsi(text);
    cout << string(midBoxWidth-1 - clean.length()/2, ' ') << bold(text);
    } else {
    cout << string(midBoxWidth-1 - text.length()/2, ' ') << bold(text);
    }

}

void SetCursorIn(int posX, int posY) {
    cout << "\033[" << posY << ";" << posX << "H";
}

void ResetCursorToInput() {
    // Teleport cursor back to the input line
    string prompt = "Enter Here: ";
    // delete the previous thingies inputs                            im gonna tweak out that 26 is for the controls
    cout << bg() << legcol() << "\33[" << consoleHeight - 3 << ";3H" << string(consoleWidth - (3+17), ' ');
    // then print cursor again
    cout << bg() << legcol() << "\33[" << consoleHeight - 3 << ";3H" << prompt;
    cout << bg() << txtcol() << "\033[" << consoleHeight - 3 << ";" << prompt.length()+3 << "H";
}

void PrintLegends(MENU m) {
  // start printing the legends/choices 5 lines from the bottom, and 3 spaces from the left
    cout << "\033[" << consoleHeight - 5 << ";1H";
    PrintLine("│", " ", "│");
    cout << "\033[" << consoleHeight - 5 << ";3H";

    string legend = "[" + bold(yellow("1")) + bg() + legcol() + "]Add "
                    "[" + bold(yellow("2")) + bg() + legcol() + "]Edit "
                    "[" + bold(yellow("3")) + bg() + legcol() + "]Delete "
                    "[" + bold(yellow("4")) + bg() + legcol() + "]Search "
                    "[" + bold(yellow("5")) + bg() + legcol() + "]Borrow "
                    "[" + bold(yellow("6")) + bg() + legcol() + "]Return "
                    "[" + bold(yellow("7")) + bg() + legcol() + "]ViewAll "
                    "[" + bold(yellow("8")) + bg() + legcol() + "]Exit";

    string legendView = "[" + bold(yellow("1")) + bg() + legcol() + "]Title "
                        "[" + bold(yellow("2")) + bg() + legcol() + "]Author "
                        "[" + bold(yellow("3")) + bg() + legcol() + "]Year "
                        "[" + bold(yellow("4")) + bg() + legcol() + "]Category "
                        "[" + bold(yellow("5")) + bg() + legcol() + "]Genre "
                        "[" + bold(yellow("6")) + bg() + legcol() + "]Status "
                        "[" + bold(yellow("7")) + bg() + legcol() + "]ID | "
                        "[" + bold(yellow("V")) + bg() + legcol() + "]iewFullBook ";

    string legendViewShort = "[" + bold(yellow("1")) + bg() + legcol() + "]Ttle "
                             "[" + bold(yellow("2")) + bg() + legcol() + "]Athr "
                             "[" + bold(yellow("3")) + bg() + legcol() + "]Year "
                             "[" + bold(yellow("4")) + bg() + legcol() + "]Cat "
                             "[" + bold(yellow("5")) + bg() + legcol() + "]Genre "
                             "[" + bold(yellow("6")) + bg() + legcol() + "]Stat "
                             "[" + bold(yellow("7")) + bg() + legcol() + "]ID │ "
                             "[" + bold(yellow("V")) + bg() + legcol() + "]iewFull ";

    string legendSearch = yellow("Global Search: " ) + bg() + txtcol() + "Enter Title/Author/Category/Genre/ID";

    string legendEdit = "[" + bold(yellow("1")) + bg() + legcol() + "]Title "
                        "[" + bold(yellow("2")) + bg() + legcol() + "]Author "
                        "[" + bold(yellow("3")) + bg() + legcol() + "]Year "
                        "[" + bold(yellow("4")) + bg() + legcol() + "]Category&Genre ";
                        // "| Leave " + txtcol() + bold("blank_") + bg() + legcol() + " to cancel";

    string legendFilter = "[" + bold(yellow("1")) + bg() + legcol() + "]Year "
                          "[" + bold(yellow("2")) + bg() + legcol() + "]Category "
                          "[" + bold(yellow("3")) + bg() + legcol() + "]Genre "
                          "[" + bold(yellow("4")) + bg() + legcol() + "]Available "
                          "[" + bold(yellow("5")) + bg() + legcol() + "]Borrowed "
                          "[" + bold(yellow("6")) + bg() + legcol() + "]OverDue ";


    string legendAdd = "Type: '" + bold(yellow("/cancel")) + bg() + legcol() + "' to cancel";
    string legendNone = "Read the boxes... Leave Blank to cancel";

    vector<string> keys = {
    " [" + yellow("N") + bg() + legcol() + "]ext ",
    " [" + yellow("B") + bg() + legcol() + "]ack ",
    " [" + yellow("S") + bg() + legcol() + "]earch",
    " [" + yellow("F") + bg() + legcol() + "]ilter",
    "e[" + yellow("X") + bg() + legcol() + "]it",

    };

    auto controls = [&](vector<string>& keys) {
      SetCursorIn(consoleWidth - 15, consoleHeight-6);
      cout << bg() << "┌─│" <<  bold(orange(" CONTROLS ")) << bg() << "│";
      for (int i = 0; i < 7; i++) {
      SetCursorIn(consoleWidth - 15, consoleHeight-5+i);
      cout << bg() << "│ " << string(13, ' ') << "│";
      }
      SetCursorIn(consoleWidth - 15, consoleHeight);
      cout << bg() << "└──────────────" << "┘";

      for (int i = 0; i < keys.size(); i++) {
      SetCursorIn(consoleWidth - 12, consoleHeight-5+i);
      cout << bg() << legcol() << keys[i];
      }


    };

    switch (m) {
      case HOME:    cout << bg() << legcol() << legend << endl;          break;
      case ADD:     cout << bg() << legcol() << legendAdd << endl;       break;
      case EDIT:    cout << bg() << legcol() << legendEdit << endl;      break;
      case RETURN:  cout << bg() << yellow(legendNone) << endl;                  break;
      case DELETE:  cout << bg() << yellow(legendNone) << endl;                  break;
      case BORROW:  cout << bg() << yellow(legendNone) << endl;                  break;
      case VIEWALL: cout << bg() << legcol() << ((consoleWidth>90) ? legendView : legendViewShort) << endl;      controls(keys);      break;
      case SEARCH:  cout << bg() << legcol() << legendSearch << endl;    controls(keys);      break;
      case FILTER:  cout << bg() << legcol() << legendFilter << endl;    controls(keys);      break;
    }

}

void PrintState(MENU m) {
  string sort = bold(orange(" Sort By: \n"));
  string opt = bold(orange(" Options: \n"));
  string edit = bold(orange(" Edit Field: \n"));
  string filter = bold(orange(" Filter By: \n"));
  string search = "";

	// yey lambdas,
  auto hint = [](string p) {
  	SetCursorIn(1, consoleHeight - 6);
	PrintLine("┌", "─", "┐");
	SetCursorIn(3, consoleHeight - 6);
	cout << yellow(p);
  };

  // clear first
  SetCursorIn(1,4);
  PrintLine("╔", "═", "╗"); // top border
  SetCursorIn(3,4);

  switch (m) {
    case ADD:     cout<<"╣"<<bold(cyan(" ADD MODE "))     <<bg()<<"╠";    			        break;
    case EDIT:    cout<<"╣"<<bold(cyan(" EDIT MODE "))    <<bg()<<"╠";   hint(edit);    break;
    case DELETE:  cout<<"╣"<<bold(cyan(" DELETE MODE "))  <<bg()<<"╠";  			          break;
    case SEARCH:  cout<<"╣"<<bold(cyan(" SEARCH MODE "))  <<bg()<<"╠";   hint(search); 	  break;
    case BORROW:  cout<<"╣"<<bold(cyan(" BORROW MODE "))  <<bg()<<"╠";  			          break;
    case RETURN:  cout<<"╣"<<bold(cyan(" RETURN MODE "))  <<bg()<<"╠";  			          break;
    case VIEWALL: cout<<"╣"<<bold(cyan(" VIEW MODE "))    <<bg()<<"╠";   hint(sort);	  break;
    case HOME:    cout<<"╣"<<bold(cyan(" HOMEPAGE "))     <<bg()<<"╠";   hint(opt);  	  break;
    case FILTER:  cout<<"╣"<<bold(cyan(" FILTER MODE "))  <<bg()<<"╠";   hint(filter);  break;
  }

}

void PrintTableHeaders() {
  cout << "\033[" << 5 << ";3H";
    // Define column width percentages (totaling 100)
    int wId       = consoleWidth * 0.09;
    int wName     = consoleWidth * 0.20;
    int wAuthor   = consoleWidth * 0.20;
    int wYear     = consoleWidth * 0.10;
    int wCategory = consoleWidth * 0.15;
    int wGenre    = consoleWidth * 0.15;
    int wStatus   = consoleWidth * 0.08;

    auto color = []() {
    return "\033[38;2;250;189;47m";
    };

    //Print with specific padding
    cout <<   left
              << color() << setw(wId)       << "ID"
              << color() << setw(wName)     << "Name"
              << color() << setw(wAuthor)   << "Author"
              << color() << setw(wYear)     << "Year"
              << color() << setw(wCategory) << "Category"
              << color() << setw(wGenre)    << "Genre"
              << color() << setw(wStatus)   << "Status"
              << color() << endl;
    PrintLine("║", "─", "║");


  ResetCursorToInput();
}

void PrintBookDetails(bool dots, const Book& b) {
    // Define column width percentages (totaling 100)
    int wId       = consoleWidth * 0.09;
    int wName     = consoleWidth * 0.20;
    int wAuthor   = consoleWidth * 0.20;
    int wYear     = consoleWidth * 0.10;
    int wCategory = consoleWidth * 0.15;
    int wGenre    = consoleWidth * 0.15;
    int wStatus   = consoleWidth * 0.09;

	string statAv = (consoleWidth < 105)  ? "Avlble" : "Available";
	string statBr = (consoleWidth < 105)  ? "Taken " : "Borrowed ";
	string statDue = (consoleWidth < 105) ? "DUE++ " : "OVERDUE  ";

  auto cleaner = [](string b, int w) {
    string highlighted = Highlight(fitTextWidth(b, w), searched);
    int cleanLen = stripAnsi(highlighted).length();
    int extraSpaces = w - cleanLen;
    if (extraSpaces < 0) extraSpaces = 0;
    return highlighted + string(extraSpaces, ' ');
  };

    if (dots) {
      cout <<   left
              << bg() << txtcol() << setw(wId)       << "..."
              << bg() << txtcol() << setw(wName)     << "..."
              << bg() << txtcol() << setw(wAuthor)   << "..."
              << bg() << txtcol() << setw(wYear)     << "..."
              << bg() << txtcol() << setw(wCategory) << "..."
              << bg() << txtcol() << setw(wGenre)    << "..."
              << bg() << txtcol() << setw(wStatus-1) << "..."
              << bg() << txtcol() << endl;
    } else {

    cout <<   left
              << bg() << txtcol() << setw(wId)       << b.ID
              << bg() << txtcol() << setw(wName)     << cleaner(b.title, wName)
              << bg() << txtcol() << setw(wAuthor)   << cleaner(b.author, wAuthor)
              << bg() << txtcol() << setw(wYear)     << cleaner(to_string(b.year), wYear)
              << bg() << txtcol() << setw(wCategory) << cleaner(b.category, wCategory)
              << bg() << txtcol() << setw(wGenre)    << cleaner(b.genre, wGenre)
              << bg() << txtcol() << setw(wStatus)   << ((b.isAvailable) ? green(statAv) : ((GetDate(today) > b.dueDate) ? bold(red(statDue)) : red(statBr)))
              << bg() << txtcol() << endl;
  }
}

void PrintPage(int& currentPage, int& totalPages) {
  string page = "╣ page " + bold(cyan(to_string(currentPage))) + bg() + " of " + to_string(totalPages) + " ╠";
  SetCursorIn(1, consoleHeight - 7);
  PrintLine("╚", "═", "╝"); // bottom border reprint for consistency
  SetCursorIn((consoleWidth/2) - (page.length()/2-36), consoleHeight - 7);
  cout << page;

  ResetCursorToInput();
}

string fitTextWidth(string text, int width) {
    string strip = stripAnsi(text);
    if (text.length() > width - 1) {
        return strip.substr(0, width - 4) + "...";
    }
    return text;
}

void PrintLine(string leftCorner, string line, string rightCorner) {
  cout << bg() << leftCorner;
  for(int i = 0; i < consoleWidth - 2; ++i) cout << line;
  cout << rightCorner;
}

string stripAnsi(string text) {
    string result;
    bool inEscape = false;

    for (char c : text) {
        if (c == '\033') {
            inEscape = true;
        }
        if (!inEscape) {
            result += c;
        }
        if (inEscape && c == 'm') {
            inEscape = false;
        }
    }

    return result;
}

string Highlight(string text, string query) {
    if (!isSearching || query.empty()) { return text; }

    string loweredText = ToLowerWord(text);
    string loweredQuery = ToLowerWord(query);

    size_t pos = loweredText.find(loweredQuery);

    if (pos == string::npos) {
        return text;
    }

    return text.substr(0, pos)
        + bold(purple(text.substr(pos, query.length())))
        + bg() + txtcol()
        + text.substr(pos + query.length());
}
