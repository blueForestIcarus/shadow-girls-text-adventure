//Author: Erich Spaker
//Class: ECE 114
//Project: final game project
//Game Name: Shadow Girl
#include <iostream>
#include <string>
#include <stdlib.h>

//timing for teletype effect
#include <chrono>
#include <thread>

/*These codes identify different types of
 * conditionals and actions for rooms in
 * the adventure game.
 */
#define CTYPE_ITEM 0
#define CTYPE_NONE 1
#define ATYPE_ENABLE 0
#define ATYPE_DISABLE 1
#define ATYPE_MOVE 2
#define ATYPE_ITEM 3
#define ATYPE_DEITEM 4
#define ATYPE_NONE 4

#define PROMPT "==> "
#define TTEXT_RATE 25

#define CLEVER_SKIP "`"
#define CLEVER_ESCAPE "%"
#define CLEVER_PROMPT ">"
#define CLEVER_CLEAR "~"
#define CLEVER_LAST "|"
#define CLEVER_INSTANT "!"
#define CLEVER_TELETYPE "@"

//Condition for option
//either use of display
typedef struct{
	int type;
	std::string detail;
} Condition;

//Action performed by an option.
typedef struct ActionT{
	int type;
	std::string detail;

	//more actions
	struct ActionT *next;
	int nextexists;
} Action;

//A selectable option in a room
typedef struct{
	int id;
	std::string text;

	//can it be used
	Condition condition;

	//should it be displayed
	Condition displaytest;

	//text printed when option used
	std::string passText;
	std::string failText;

	//this is what the option does
	Action action;

	int disable_on_use;
	int used;
	int failed;

	int enabled;


} Option;

//A room in the adventure game
typedef struct{
	std::string id;
	std::string text;
	Option* options;
	int num_options;
	int passes;
} Room;

//inventory items
typedef struct{
	std::string id;
	std::string text;
	int quantity;
} Item;

Item NullItem = {"", "Not everything exists.", 0};
Room NullRoom = {"", "Not everywhere exists.", NULL, 0};

//contains all game data
typedef struct{
	std::string current;

	Item* items;
	int num_items;

	Room* rooms;
	int num_rooms;
} Config;

Config data;

//called by main setup game
Config loadAllData();

//ascii art of a heart for the end
std::string ascii_heart = " ________________ \n|      _  _      |\n|   \\ ( \\/ ) /   |\n|  --  \\  ";
std::string endl = "\n";

//fun error messages if you enter something incorrectly
std::string error_messages[] = {"Afraid thats not an option pal.", "When you come to a fork in the road, take it.", "I know what an option look like and that ain't it.", "Sometimes we have to make a choice among limited options.", "When life gives you lemons...", "You're gunna have to learn to make do with what options you have.", "You think you can do these things but you just can't nemo."};
int num_error_messages = 7;

//print instantly
void itext(std::string text){
	std::cout << text;
}

//wait <ms> milliseconds
void pause(int ms){
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#include <cstdlib>

//clear the console window, found on stackoverflow
//https://stackoverflow.com/questions/228617/how-do-i-clear-the-console-in-both-windows-and-linux-using-c
void clear(){
#ifdef WINDOWS
	//this is windows
    std::system("cls");
#else
    //this is mac/linux
    std::system("clear");
#endif
}

//get input
std::string prompt(){
	itext(PROMPT);
	//wait for enter key
	std::string text;
	std::getline(std::cin, text);
	return text;
}

//wait for user to press enter
void wait(){
	//wait for enter key
	std::getchar();
}

//print text all cool like
void ttext(std::string text){
	int i = 0;
	while(text[i]!= '\0'){
		std::cout << text[i];
		pause(TTEXT_RATE);
	}
}

//print text then pause
void ttextp(std::string text){
	//print text
	ttext(text);
	wait();
}

//prints text as a quote
void qttexp(std::string text, std::string person){
	itext(person + ": ");
	ttextp("\""+text+"\"");
}

//print text the smart way
void clevertext(std::string text,bool used){
	//fuck this shit todo
}

//print errors for debugging
void error(std::string s){
	ttext("Dear Programmer: " + s + "\n");
}

Room* getroom(std::string s){
	for(int i = 0 ; i<data.num_rooms ; i++){
		if(data.rooms[i].id==s){
			return &(data.rooms[i]);
		}
	}

	error("take a hard long look at where you think you're going.");
	return &NullRoom;
}

Item* getitem(std::string s){
	for(int i = 0 ; i<data.num_items ; i++){
		if(data.items[i].id==s){
			return &(data.items[i]);
		}
	}

	error("you are searching for something which cannot be had.");
	return &NullItem;
}

Option* getoption(std::string r, std::string s){
	for(int i = 0 ; i<data.num_rooms ; i++){
		if(data.rooms[i].id==r){
			for(int o = 0 ; o<data.rooms[i].num_options ; o++){
				if(data.rooms[i].options[o].id==s){
					return &(data.rooms[i].options[o]);
				}
			}
		}
	}

	error("some opportunities are missed no matter where you are.");
	return &NullItem;
}

//check if player has item in inventory
bool playerhas(std::string s){
	return getitem(s)->quantity > 0;
}

//give item to player
int playergive(std::string s, int a){
	getitem(s)->quantity += a;
	return getitem(s)->quantity;
}

//set current room
bool setroom(std::string s){
	if(getroom(s)->id == ""){
		data.current = s;
		return true;
	}

	return false;
}

//check if player has all heart fragments
void gameover(){
	if(playerhas("HF1") &&
		playerhas("HF1") &&
		playerhas("HF1") &&
		playerhas("HF1") &&
		playerhas("HF1")
	){
		ttextp("Congratulations.");
		ttextp("You found all the heart fragments.");
		ttextp( ascii_heart + "The glow is so bright...");
		ttextp("Maybe... too bright.");
		ttextp("The light, it's drowning you out.");
		ttextp("I can hardly see you.");
		ttextp("Shadow girl?");
		ttextp(".");
		ttextp("..");
		ttextp("...");
		ttextp("I guess you can't speak anymore.");
		ttextp("I guess, with your heart back, you're just a normal shadow again.");
		ttextp("It was fun, helping you.");
		ttextp("Take good care of that heart, okay?");
		ttextp("I know you will do something great with it.");
		ttext("Goodbye shadow girl.\n");
		pause(4000);
		while(true) ttextp("CLOSE TERMINAL TO EXIT");
	}
}

int loop(bool fast){
	//display room text.
	Room* current = getroom(data.current);
	if(fast){
		clevertext(CLEVER_SKIP+current->text,current->passes>0);
	}else{
		clevertext(current->text,current->passes>0);
	}

	//display options.
	int options[100];
	int num_options=0;

	for(int i = 0 ; i < current->num_options ; i++){
		bool spent = current->options[i].used > 0 && current->options[i].disable_on_use;
		if(current->options[i].enabled && !spent){//todo display condition
			itext(std::to_string(num_options+1) + current->options[i].text + endl);
			pause(TTEXT_RATE*2);
			options[num_options]=i;
			num_options++;
		}
	}

	//get choice.
	int select = 0;
	while(true){
		std::string input = prompt();
		try{
			select = std::stoi(input)-1;
			if(select < 0 || select >= num_options) throw;
			select = options[select];
			break;
		}catch(...){
			ttext(error_messages[rand() % num_error_messages] + "\n");
		}
	}
	Option* choosen = &(current->options[select]);

	//print option text
	clevertext(choosen->text, choosen->failed + choosen->used);

	//check condition
	bool pass;
	switch(choosen->condition.type){
		case CTYPE_NONE:
			pass=true;
			break;

		case CTYPE_ITEM:
			pass=playerhas(choosen->condition.detail);
			break;

		default:
			pass=false;
			error("I'm not sure what you are asking.");
			break;
	}

	if(pass){
		clevertext(choosen->passText,choosen->used);
	}else{
		clevertext(choosen->failText,choosen->failed);
		choosen->failed++;
		wait();
		return true;
	}

	//preform action
	Action action = choosen->action;
	while(true){
		switch(action.type){
			case ATYPE_NONE:
				break;
			case ATYPE_ITEM:
				playergive(action.detail,1);
				break;
			case ATYPE_DEITEM:
				playergive(action.detail,-1);
				break;
			case ATYPE_ENABLE:
				std::string roomcode = action.detail.substr(0, action.detail.find(":"));
				std::string optcode = action.detail.substr(action.detail.find(":") + 1);
				getoption(roomcode,optcode)->enabled = true;
				break;
			case ATYPE_DISABLE:
				std::string roomcode = action.detail.substr(0, action.detail.find(":"));
				std::string optcode = action.detail.substr(action.detail.find(":") + 1);
				getoption(roomcode,optcode)->enabled = false;
				break;
			case ATYPE_MOVE:
				setroom(action.detail);
				break;
			default:
				error("I can only do what I have been taught to do.");
				break;
		}

		if(action.nextexists){
			action = *action.next;
			continue;
		}
		break;
	}

	choosen->used++;

	//check to make sure everything is okay
	if(getroom(data.current)->options[select].used == 0){
		error("assignment ineffective... welcome to pointer hell.");
	}

	//give user time to read
	wait();

	//check for endgame
	gameover();

	return false;
}

int main(){
	data = loadAllData();

	bool fast = false;
	while(true){fast = loop(fast);}
	return 0;
}

Config loadAllData(){
	Config stuff;
	//todo
	return stuff;
}
