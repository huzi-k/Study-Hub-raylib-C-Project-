#include "raylib.h"
#include<iostream>
#include <fstream>
#include <string>
using namespace std;

// -------------------------
// Screen Base Class
// -------------------------
class TitleScreen;
class Task;
class Calendar;
class Clock;
class LockIn;

class screen {
public:
    virtual void Update() = 0;                  //function overloading
    virtual void Draw() = 0;
};

screen* ptr = nullptr; // global ptr
Music bgMusic;                                  //global so that all classes can access


// Factory
screen* createTitleScreen();            //has to be declared first so that classes declared before Class Titlescreen can change its ptr to this
const int row = 5;
const int col = 7;
const int  calsize = 10;
const int boxsize = 70;
Vector2 gridorgin;

// -------------------------
// CLOCK 
// -------------------------


void clockDraw() {
    // for the clock position on screen
    Vector2 center = { 720.0f, 80.0f };
    float radius = 70.0f;


    DrawCircleV(center, radius, LIGHTGRAY);//outline circle of clock
    DrawCircleLines((int)center.x, (int)center.y, radius, BLACK);//pivot point

    //12 marks which are speced at every 30 degress
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * (3.14159265f / 180.0f) - 3.14159265f / 2; //-90 shifts 
        Vector2 p1 = { center.x + cosf(angle) * (radius - 10), center.y + sinf(angle) * (radius - 10) };
        Vector2 p2 = { center.x + cosf(angle) * (radius - 25), center.y + sinf(angle) * (radius - 25) };
        DrawLineEx(p1, p2, 4.0f, BLACK);
    }
    time_t t = time(NULL);
    time_t now = time(0);// Get current system time 
    tm ltm;
    localtime_s(&ltm, &now);//Convert raw time into structured time
    int hour = ltm.tm_hour;
    int minute = ltm.tm_min;
    int second = ltm.tm_sec;

    //smootrh movement of clock hands 
    float smoothSec = second + (GetTime() - floor(GetTime()));

    // this gives us angle in degrees
    float hourAngle = ((hour % 12) + minute / 60.0f + smoothSec / 3600.0f) * 30.0f - 90.0f;
    float minAngle = (minute + smoothSec / 60.0f) * 6.0f - 90.0f;
    float secAngle = smoothSec * 6.0f - 90.0f;

    // length of hands
    float hourLen = radius * 0.5f;
    float minLen = radius * 0.75f;
    float secLen = radius * 0.9f;

    // this compute endspoints from angle for hands
    Vector2 hourP = { center.x + cosf(hourAngle * 3.14159265f / 180.0f) * hourLen,
                      center.y + sinf(hourAngle * 3.14159265f / 180.0f) * hourLen };
    Vector2 minP = { center.x + cosf(minAngle * 3.14159265f / 180.0f) * minLen,
                      center.y + sinf(minAngle * 3.14159265f / 180.0f) * minLen };
    Vector2 secP = { center.x + cosf(secAngle * 3.14159265f / 180.0f) * secLen,
                      center.y + sinf(secAngle * 3.14159265f / 180.0f) * secLen };

    //drawing the actual hands
    DrawLineEx(center, hourP, 8.0f, BLACK);
    DrawLineEx(center, minP, 5.0f, BLACK);
    DrawLineEx(center, secP, 2.0f, RED);

    DrawCircleV(center, 6.0f, BLACK);

    // to draw the digital timer under the clock
    DrawText(TextFormat("%02d:%02d:%02d", hour, minute, second), 660, 155, 32, WHITE);
}


// -------------------------
// TASK PAGE
// -------------------------
class Node { //linked list for saving tasks
public:
    string title;
    string due;
    Node* next;
    Node(const string s, const string d)
    {
        title = s;
        due = d;
        next = nullptr;
    }
};

Node* head = nullptr;

// LOADING AND DRAIWNG TASKS 
void DrawTasks(Node* head)
{

    Node* temp = head;
    int x = 120;
    int y = 325;
    while (temp != nullptr)
    {
        DrawText(temp->title.c_str(), x, y, 20, BLACK); //drawing title and due date of task
        DrawText(temp->due.c_str(), x, y + 20, 20, BLACK);
        y = y + 50;     //increaseing y val by 50 to space out tasks
        temp = temp->next;
    }
}
// loading from file via linked lists
Node* loadfromFile()
{
    ifstream infile("tasks.txt");
    string title;
    string duedate;

    head = nullptr;
    Node* tail = nullptr;

    while (true)
    {
        // read title
        if (!getline(infile, title))
            break;

        // read due date
        if (!getline(infile, duedate))
            break;

        Node* n = new Node(title, duedate);

        if (head == nullptr)
        {
            head = n;
            tail = n;
        }
        else
        {
            tail->next = n;  // correct link
            tail = n;
        }
    }

    return head; //returning head of linked list (first task title and deu date)
}



class Task : public screen {
    //loading textures
    Texture2D back;
    Texture2D todoback;
    Texture2D backarrow, removeicon, addicon, list;
    //two bools to check if add or remove task is clicked
    bool addtask = false;
    bool remove = false;
    string titleinput = ""; //enmpty strings to add title and due date
    string duedateinput = "";
    const int MAX_INPUT_CHARS = 50; // max characters
    // two bools to check if typing in title or due date box
    bool typingtitle = false;
    bool typyingdd = false;
    int framesCounter = 0; //frame counter for input boxes (blinking effect kai liay use kartay)
    // bool to check if remove task is clicked
    bool removetsk = false;
    //Node* head;
    //making the linked list

public:
    Task() {
        back = LoadTexture("resources/background.png");
        backarrow = LoadTexture("resources/arrow.png");
        removeicon = LoadTexture("resources/subtract.png");
        addicon = LoadTexture("resources/add.png");
        todoback = LoadTexture("resources/todo.png");

    }
    //save the linked list to file in a manner that can be read back and stored in linked list
    void savetask(const string& title, const string& dd)
    {
        Node* n = new Node(title, dd);
        if (head == nullptr)
        {
            head = n;
        }
        else
        {
            Node* temp = head;
            while (temp->next != nullptr)
            {
                temp = temp->next;
            }
            temp->next = n;
        }
        SaveToFile();

    }
    //remove task from linked list via title input and then calling the save to file function to update the file
    void removetask(const string& title)
    {
        Node* temp = head;
        Node* prev = nullptr;
        while (temp != nullptr && temp->title != title)
        {
            prev = temp;
            temp = temp->next;
        }
        if (temp == nullptr)
        {
            return;
        }
        if (prev == nullptr)
        {
            head = temp->next;
        }
        else
            prev->next = temp->next;
        delete temp;
        SaveToFile();

    }
    //save to file func jaisay linked list ko normal save karwatay 
    void SaveToFile() {
        ofstream file("tasks.txt");
        Node* temp = head;
        while (temp != nullptr)
        {
            file << temp->title << endl << temp->due << endl;
            temp = temp->next;
        }

    }
    //iterate linked list to count tasks
    int counttask() {
        int count = 0;
        Node* temp = head;
        while (temp != NULL) {
            count++;
            temp = temp->next;
        }
        return count;
    }

    void Update() {


        Vector2 mouse = GetMousePosition(); //struct from raylib that holds two flaoting point used here to get mouse position
        // creating a backbutton starting from top left and is 25% of actual size of image/graphic uploaded
        Rectangle BackBtn{ 0,0,(float)backarrow.width * 0.25f,(float)backarrow.height * 0.25f };

        //creating add and remove buttons with their positions and sizes
        Rectangle addbtn = { 619, 315,(float)addicon.width * 0.25f,(float)addicon.height * 0.25f };
        Rectangle removebtn = { 610, 510,(float)removeicon.width * 0.25f,(float)removeicon.height * 0.25f };

        if (CheckCollisionPointRec(mouse, BackBtn) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { //checks if backbtn is clicked
            if (addtask == false)
            {
                ptr = createTitleScreen(); //if yes then goes back to title screen
            }
        }
        if (CheckCollisionPointRec(mouse, addbtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) //checks if addbtn is clicked
        {
            addtask = true; //if yes then the add task bool is true
        }
        if (CheckCollisionPointRec(mouse, removebtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) //checks if backbtn is clicked
        {
            removetsk = true;
        }
        if (addtask == true) //when addtask bool is tru ( when add task is clicked) 
        {

            //making 2 clickable boxes
            Rectangle titlebox = { 150,235,350,40 };
            if (CheckCollisionPointRec(mouse, titlebox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                typingtitle = true; //bool turns true indicating that we are now typing title
                typyingdd = false;
            }
            Rectangle duebox = { 150, 340, 250 ,40 };
            if (CheckCollisionPointRec(mouse, duebox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                typingtitle = false;
                typyingdd = true; //bool turns true indicating that we are now typing due date while typing title remains false
            }

            // now adding txt via box 
            // Determine which input box is active, wo bool true hogai ga coz checkcollision func returns true if mouse is inside the box
            bool mouseOnTitle = CheckCollisionPointRec(mouse, titlebox);
            bool  mouseOnDue = CheckCollisionPointRec(mouse, duebox);


            SetMouseCursor((mouseOnTitle || mouseOnDue) ? MOUSE_CURSOR_IBEAM : MOUSE_CURSOR_DEFAULT);
            //if mouse is inside any box then change cursor to text input ( | )  cursor else default cursor

            // Get typed characters
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32 && key <= 125))
                { // printable chars
                    if (typingtitle && titleinput.size() < MAX_INPUT_CHARS) //if typing title is true and size of title string is less than max chars
                        titleinput.push_back((char)key);  //then add the char in the title string same neechay for due date
                    else if (typyingdd && duedateinput.size() < MAX_INPUT_CHARS)
                        duedateinput.push_back((char)key);
                }
                key = GetCharPressed();
            }

            // Backspace
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (typingtitle && !titleinput.empty())
                {
                    titleinput.pop_back(); //if bakcspace is clicked and title typing is true and title string is not empty then remove last char
                }
                if (typyingdd && !duedateinput.empty())
                {
                    duedateinput.pop_back();
                }
            }

            framesCounter++;
            //making the save and cancel butoons fucntionalbe

            Rectangle saveBtn = { 250, 400, 200, 50 };
            if (CheckCollisionPointRec(mouse, saveBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                savetask(titleinput, duedateinput); //saving in a linked list
                titleinput.clear(); //clearing the strings and due date and setting the flags back to the original
                duedateinput.clear();
                typingtitle = false;
                typyingdd = false;
                return;
            }
            Rectangle cancelBtn = { 250, 470, 200, 50 };
            if (CheckCollisionPointRec(mouse, cancelBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                titleinput.clear(); //clearing the strings and due date and setting the flags back to the original without saving
                duedateinput.clear();
                addtask = false;
                typingtitle = false;
                typyingdd = false;
            }

            return;
        }

        if (removetsk == true)
        {
            Rectangle titlebox = { 150,235,350,40 };
            if (CheckCollisionPointRec(mouse, titlebox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                typingtitle = true;

            }
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32 && key <= 125) && typingtitle && titleinput.size() < MAX_INPUT_CHARS)
                {
                    titleinput.push_back((char)key);
                }
                key = GetCharPressed();
            }

            // Backspace
            if (IsKeyPressed(KEY_BACKSPACE))
            {

                titleinput.pop_back();
            }
            framesCounter++;
            Rectangle confirmRemoveBtn = { 250, 400, 200, 50 };
            if (CheckCollisionPointRec(mouse, confirmRemoveBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                removetask(titleinput); //removing task from linked list by calling remove task function and passing title string
                titleinput.clear();
                removetsk = false; // close remove screen
                typingtitle = false;
                return;
            }

            // CANCEL button
            Rectangle cancelRemoveBtn = { 250, 470, 200, 50 };
            if (CheckCollisionPointRec(mouse, cancelRemoveBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                titleinput.clear();
                removetsk = false;
                typingtitle = false;
            }

            return;

        }
    }

    void Draw() //used to draw the page
    {
        DrawTexture(todoback, 0, 0, WHITE);
        //DrawTexture(back, 0, 0, WHITE); //basic tecture ( backgrnd)
        DrawTextureEx(backarrow, { 0,0 }, 0, 0.25f, WHITE); //gives more control over what texture is drawn

        DrawText("Task Page", 240, 275, 48, BLACK);
        DrawTextureEx(addicon, { 619, 315 }, 0.25f, 0.25f, WHITE);
        DrawText("Add ", 635, 460, 35, BLACK);
        DrawTextureEx(removeicon, { 610, 510 }, 0.25f, 0.25f, WHITE);
        DrawText("Remove", 610, 650, 35, BLACK);

        /* loading and displaying from file*/
        Node* task = loadfromFile();
        DrawTasks(task);
        if (addtask == true)
        {
            DrawRectangle(130, 130, 420, 500, BROWN);
            DrawText("Add new Task:", 150, 150, 40, WHITE);

            DrawText("Title:", 150, 200, 30, WHITE);
            DrawRectangle(150, 235, 350, 40, DARKGRAY);
            DrawText(titleinput.c_str(), 150, 235, 25, WHITE);

            DrawText("Due (DD/MM/Y):", 150, 300, 30, WHITE);
            DrawRectangle(150, 340, 250, 40, DARKGRAY);
            DrawText(duedateinput.c_str(), 150, 340, 25, WHITE);//this line shows the entered char

            // Save button
            DrawRectangle(250, 400, 200, 50, GREEN);
            DrawText("SAVE", 310, 415, 30, WHITE);

            // Cancel button
            DrawRectangle(250, 470, 200, 50, RED);
            DrawText("CANCEL", 290, 485, 30, WHITE);

            //for the blinking affect
            framesCounter++;

            // Title cursor
            if (typingtitle && ((framesCounter / 20) % 2) == 0)
                DrawText("_", 150 + MeasureText(titleinput.c_str(), 25), 235, 25, WHITE);

            // Due cursor
            if (typyingdd && ((framesCounter / 20) % 2) == 0)
                DrawText("_", 150 + MeasureText(duedateinput.c_str(), 25), 340, 25, WHITE);
        }
        if (removetsk == true)
        {
            if (removetsk == true)
            {
                DrawRectangle(130, 130, 420, 500, BROWN);
                DrawText("Remove Task:", 150, 150, 40, WHITE);

                DrawText("Title:", 150, 200, 30, WHITE);
                DrawRectangle(150, 235, 350, 40, DARKGRAY);
                DrawText(titleinput.c_str(), 150, 235, 25, WHITE);

                DrawRectangle(240, 400, 190, 50, MAROON);
                DrawText("REMOVE", 275, 415, 30, WHITE);

                DrawRectangle(240, 470, 190, 50, GRAY);
                DrawText("CANCEL", 275, 485, 30, WHITE);

                if (typingtitle && ((framesCounter / 20) % 2) == 0)
                    DrawText("_", 150 + MeasureText(titleinput.c_str(), 25), 235, 25, WHITE);
            }
        }
    }
};


/*----------------------------------
  HELPER FUNCTIONS FOR THE CALANDER CLASS
------------------------------------*/

void parse(int& day, int& month, int& year, bool& condition, int num, int monthnum) {

    ifstream in("tasks.txt");
    string line;
    char slash1, slash2;
    while (getline(in, line)) {
        //this function seperates the day, month, year from  the task.txt file written in day/month/year format
        if (!(in >> day >> slash1 >> month >> slash2 >> year)) {
            break;
        }
        getline(in, line); // to skip a title line
        if (day == num && month == monthnum + 1) {
            condition = true;                                                                //checks the day and month and if it matches with the day and month written in the task.txt file. if so any condition is true
            break;
        }
    }
}

string gettitle(int& day, int& month, int& year, int num, int monthnum) {

    ifstream in("tasks.txt");
    string title;
    string line;
    char slash1, slash2;
    while (getline(in, title)) {

        if (!(in >> day >> slash1 >> month >> slash2 >> year)) {                            // this does the same except it returns the title above the dates in the task.txt file
            break;
        }
        getline(in, line);
        if (day == num && month == monthnum + 1) {                                  //checks the day and month and if it matches with the day and month written in the task.txt file. if so any condition is true
            return title;
        }
    }

    return "";
}


// -------------------------
// CALENDAR PAGE
// -------------------------
class Calendar : public screen {
    Texture2D back, backarrow, rightarr, leftarr;
    char cal[row][col];
    bool showtask;
    int showday;
    int showMonth;
    int currMonth;
    string Months[12] = { "January","Febuary","March","April","May","June","July","August","September","October","November","December" };  // array of months to be shown

public:
    Calendar() {
        showtask = false;
        currMonth = 11;             // sets month to december (11th in the Months array)
        back = LoadTexture("resources/calander.png");
        backarrow = LoadTexture("resources/arrow.png");             //loads the textures used on the page
        rightarr = LoadTexture("resources/right.png");
        leftarr = LoadTexture("resources/left.png");

    }

    void Update() {
        Vector2 mouse = GetMousePosition();
        Rectangle BackBtn{ 0,0,(float)backarrow.width * 0.25f,(float)backarrow.height * 0.25f };
        Rectangle right{ 680, 420,(float)rightarr.width * 0.15f, (float)rightarr.height * 0.15f };          // creates a button places them on top of the textures
        Rectangle left{ 70, 420,(float)leftarr.width * 0.15f, (float)leftarr.height * 0.15f };

        if (CheckCollisionPointRec(mouse, right) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currMonth++;                                                                                // if the right arrow is pressed the current month increases by one
            if (currMonth > 11) {                                                                       // if current month exceeds the array length goes back to start of the array (january)                 
                currMonth = 0;
            }
        }
        if (CheckCollisionPointRec(mouse, left) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currMonth--;                                                                              // if the left arrow is pressed the current month increases by one
            if (currMonth < 0) {                                                                      //if current month goes below the base length of the array goes to the end of the array (december)
                currMonth = 11;
            }
        }

        int daycounter = 1;
        if (CheckCollisionPointRec(mouse, BackBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ptr = createTitleScreen();                                                                  //exit goes back to title
        }
        for (int x = 0; x < row; x++) {
            for (int y = 0; y < col; y++) {
                Rectangle framebtn{ 160 + (y * boxsize) , 275 + (x * boxsize),boxsize,boxsize };
                int day, month, year;
                bool drawnbtn = false;
                parse(day, month, year, drawnbtn, daycounter, currMonth);
                if (CheckCollisionPointRec(mouse, framebtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    showMonth = currMonth;
                    showday = daycounter;                                                    // a button on each square box if clicked will set the showMonth and showday and set showtask to true (hence drawing it on page)
                    showtask = true;
                }
                daycounter++;
            }

        }

    }

    void Draw() {
        DrawTextureEx(back, { 0, 0 }, 0, 1.1f, WHITE);
        DrawTextureEx(backarrow, { 0,0 }, 0, 0.25f, WHITE);
        DrawTextureEx(rightarr, { 680, 420 }, 0, 0.15f, WHITE);
        DrawTextureEx(leftarr, { 70, 420 }, 0, 0.15f, WHITE);                       //draws the textures
        DrawText("Calendar Page", 290, 60, 30, BLACK);
        DrawText(Months[currMonth].c_str(), 220, 90, 80, BLACK);



        int daycounter = 1;                        //very important variable goes through each day in loop
        for (int x = 0; x < row; x++) {
            for (int y = 0; y < col; y++) {
                Rectangle frame{ 160 + (y * boxsize) , 275 + (x * boxsize),boxsize,boxsize };
                DrawRectangleRec(frame, WHITE);                                                            //using 2d arrays to create the table    -- frame and box size are const global variables defined at the top
                DrawRectangleLinesEx(frame, 1, BLACK);                                                     // they just set the size of the table
                string count = to_string(daycounter);
                Vector2 textSize = MeasureTextEx(GetFontDefault(), count.c_str(), 20, 1);
                Vector2 pos{
                    frame.x + (frame.width - textSize.x) / 2,                               // setting the size of the numbers that will be in the boxes
                    frame.y + (frame.height - textSize.y) / 2
                };
                int day, month, year;
                bool highlight = false;
                parse(day, month, year, highlight, daycounter, currMonth);             // important is that we pass the daycounter (current day in loop) and the current month which will be checked if matches with task.txt

                if (highlight) {
                    DrawRectangle(160 + (y * boxsize), 275 + (x * boxsize), boxsize, boxsize, GREEN);
                }
                if (daycounter >= 1 && daycounter <= 31) {
                    DrawTextEx(GetFontDefault(), count.c_str(), pos, 20, 1, BLACK);             //  prints the numbers in the boxes
                }
                daycounter++;  // increase number

            }

        }
        if (showtask) {
            DrawRectangle(230, 638, 350, 50, BROWN);
            int tempday, tempmonth, tempyear;
            bool found;
            //if button is pressed and showTask is true, it will draw a box with the task details on the bottom
            string title = "Task: " + gettitle(tempday, tempmonth, tempyear, showday, showMonth);

            DrawText(title.c_str(), 235, 650, 30, WHITE);

        }

    }
};



// -------------------------
// LOCK-IN PAGE
// -------------------------
class LockIn : public screen {
    Texture2D back, backarrow, play, leftarr, rightarr;
    int musiccount;
    string music[3] = { "resources/lofimusic.mp3","resources/lofimusic1.mp3","resources/lofimusic2.mp3" };      // locations of 3 music files in array
public:
    LockIn() {
        back = LoadTexture("resources/lofi.png");
        backarrow = LoadTexture("resources/arrow.png");
        play = LoadTexture("resources/lofigirlmusic.png");

        musiccount = 0;             // sets the music to the first location in the music array

    }

    void Update() {
        Vector2 mouse = GetMousePosition();
        Rectangle BackBtn{ 0,0,(float)backarrow.width * 0.25f,(float)backarrow.height * 0.25f };
        Rectangle forwardbtn{ 470, 520,70, 70 };                                                                //buttons on the exit forward and back on the music player
        Rectangle prevbtn{ 260, 520,70, 70 };


        if (CheckCollisionPointRec(mouse, BackBtn) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {          //exit back to title screen
            ptr = createTitleScreen();
        }
        if (CheckCollisionPointRec(mouse, forwardbtn) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            musiccount++;                                           // if forward button pressed music count will increase by one hence in the array loads the next file
            if (musiccount > 2) {                                   // if music count exceeds size of array sets it to the first index
                musiccount = 0;
            }
            StopMusicStream(bgMusic);
            UnloadMusicStream(bgMusic);
            bgMusic = LoadMusicStream(music[musiccount].c_str());               //unloads current music and plays the incremented musiccount 
            PlayMusicStream(bgMusic);
        }
        if (CheckCollisionPointRec(mouse, prevbtn) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            musiccount--;                                          //if previous button pressed music count will decrease by one hence in the array loads the previous file
            if (musiccount < 0) {                                  // if music count goes below the array size it will set it to the max index
                musiccount = 2;
            }
            StopMusicStream(bgMusic);
            UnloadMusicStream(bgMusic);
            bgMusic = LoadMusicStream(music[musiccount].c_str());           //again unloads and loads new musiccount 
            PlayMusicStream(bgMusic);
        }
    }

    void Draw() {
        DrawTextureEx(back, { 0, 0 }, 0, 1.1f, WHITE);
        DrawTexture(play, 0, 0, WHITE);
        DrawTextureEx(backarrow, { 0,0 }, 0, 0.25f, WHITE);             //draws textures
        DrawText("Study Session", 200, 50, 48, WHITE);
        clockDraw();


    }
};


// -------------------------
// TITLE SCREEN
// -------------------------
class TitleScreen : public screen {
    Texture2D back;
    Texture2D taskIcon;
    Texture2D calIcon;
    Texture2D lockIcon;
    Texture2D exitIcon;
    Texture2D dashIcon;

public:
    TitleScreen() {
        back = LoadTexture("resources/main.png");
        taskIcon = LoadTexture("resources/Task.png");
        calIcon = LoadTexture("resources/calen.png");
        lockIcon = LoadTexture("resources/lockin.png");
        exitIcon = LoadTexture("resources/exiticon.png");

    }

    void Update() {
        Vector2 mouse = GetMousePosition();

        Rectangle taskBtn = { 100,250,(float)taskIcon.width * 0.17f,(float)taskIcon.height * 0.17f };
        Rectangle calBtn = { 285,250,(float)calIcon.width,(float)calIcon.height };
        Rectangle lockBtn = { 525,260,(float)lockIcon.width * 0.35f,(float)lockIcon.height * 0.35f };
        Rectangle exitBtn = { 0, 0, exitIcon.width * 0.265f, exitIcon.height * 0.265f };
        Rectangle DashBtn = { 325,470 , dashIcon.width * 0.18f , dashIcon.height * 0.18f };

        if (CheckCollisionPointRec(mouse, taskBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ptr = new Task;            //if the add task button is clicked, it will point the pointer to the task screen
        }

        if (CheckCollisionPointRec(mouse, calBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ptr = new Calendar;
        }

        if (CheckCollisionPointRec(mouse, lockBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ptr = new LockIn;
        }

        if (CheckCollisionPointRec(mouse, exitBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            exit(0);
        }

    }

    void Draw() {
        DrawTextureEx(back, { 0, 0 }, 0, 1.1, WHITE);
        DrawText("STUDY HUB", 150, 100, 72, BLACK);
        DrawTextureEx(exitIcon, { 0,0 }, 0, 0.265f, WHITE);
        DrawTextureEx(taskIcon, { 100, 250 }, 0, 0.17f, WHITE);
        DrawTexture(calIcon, 285, 250, WHITE);
        DrawTextureEx(lockIcon, { 525, 260 }, 0, 0.35f, WHITE);
        clockDraw();
    }
};

// -------------------------
// FACTORY METHODS
// -------------------------
screen* createTitleScreen() { return new TitleScreen(); }

// -------------------------
// MAIN
// -------------------------
int main() {
    const int screenWidth = 800;
    const int screenHeight = 700;
    InitWindow(screenWidth, screenHeight, "Study UI");

    // Load custom cursor
    Texture2D cursor = LoadTexture("resources/pencil.png");
    HideCursor(); // hide system cursor

    InitAudioDevice();
    bgMusic = LoadMusicStream("resources/lofimusic.mp3");
    PlayMusicStream(bgMusic);

    SetTargetFPS(60);
    ptr = createTitleScreen();

    while (!WindowShouldClose()) {
        UpdateMusicStream(bgMusic);
        ptr->Update();

        BeginDrawing();           // drawing begins
        ClearBackground(BLACK);

        ptr->Draw();              // draw all UI first

        // Draw custom cursor LAST, inside Begin/EndDrawing
        Vector2 mouse = GetMousePosition();
        DrawTextureEx(cursor, mouse, 0.0f, 0.2f, WHITE); // 40% size

        EndDrawing();             // drawing ends
    }

    UnloadTexture(cursor);
    CloseWindow();
    return 0;
}