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
#include <cstdlib>

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
#define ATYPE_NONE 5

#define PROMPT "==> "
#define TTEXT_RATE 20

#define CLEVER_SKIP '`'
#define CLEVER_ESCAPE '%'
#define CLEVER_PROMPT '>'
#define CLEVER_CLEAR '~'
#define CLEVER_LAST '|'
#define CLEVER_INSTANT '!'
#define CLEVER_TELETYPE '@'
//todo line by line teletype

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
	std::string id;
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
Option NullOption = {"", "Some things aren't a choice."};

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
void loadAllData();

//ascii art of a heart for the end
std::string ascii_heart = " ________________ \n|      _  _      |\n|   \\ ( \\/ ) /   |\n|  --  \\  ";
std::string endl = "\n";

//fun error messages if you enter something incorrectly
std::string error_messages[] = {"Afraid thats not an option pal.", "When you come to a fork in the road, take it.", "I know what an option look like and that ain't it.", "Sometimes we have to make a choice among limited options.", "When life gives you lemons...", "You're gunna have to learn to make do with what options you have.", "You think you can do these things but you just can't nemo."};
int num_error_messages = 7;

//print instantly
void itext(std::string text){
	std::cout << text  << std::flush;
}

//wait <ms> milliseconds
void pause(int ms){//todo test this
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

//clear the console window, found on stackoverflow
//https://stackoverflow.com/questions/228617/how-do-i-clear-the-console-in-both-windows-and-linux-using-c
void clear(){//todo test
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
	for(int i=0 ; i< (signed)text.length() ; i++){
		std::cout << text[i] << std::flush;
		pause(TTEXT_RATE);
	}
}

//print text then pause
void ttextp(std::string text){
	ttext(text);
	wait();
}

//prints text as a quote
void qttexp(std::string text, std::string person){
	itext(person + ": ");
	ttextp("\""+text+"\"");
}

void clevertextflush(std::string buffer, bool prompt, bool clr, bool teletype){
	if(teletype){
		ttext(buffer);
	}else{
		itext(buffer);
	}

	if(prompt){
		wait();
	}

	if(clr){
		clear();
	}
}

//print text the smart way
void clevertext(std::string text,bool used){
	//why did I decide this was necessary?
	bool escaped = false;
	bool waiting = used;
	std::string buffer = "";
	bool skip = text[0]==CLEVER_SKIP;//todo
	bool teletype = true;

	for(int i = 0 ; i < (signed) text.length() ; i++){
		if(escaped){
			buffer += text[i];
			escaped = false;
			continue;
		}

		if(waiting){
			if(text[i]==CLEVER_LAST){
				waiting = false;
				buffer = "";
			}
			continue;
		}

		switch(text[i]){
			case CLEVER_ESCAPE:
				escaped = true;
				break;

			//these are implemented above
			case CLEVER_LAST:
				break;
			case CLEVER_SKIP:
				break;

			//these require flushing the buffer
			case CLEVER_PROMPT:
				clevertextflush(buffer, true,false,teletype);
				buffer = "";
				break;
			case CLEVER_CLEAR:
				clevertextflush(buffer, true,true,teletype);
				buffer = "";
				break;
			case CLEVER_INSTANT:
				clevertextflush(buffer, false,false,teletype);
				buffer = ""; teletype = false;
				break;
			case CLEVER_TELETYPE:
				clevertextflush(buffer, false,false,teletype);
				buffer = ""; teletype = true;
				break;
			default:
				buffer+=text[i];
				break;
		}

	}
	if(waiting){
		clevertext(text,false);
	}else{
		clevertextflush(buffer,false, false,teletype);
	}
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
	return &NullOption;
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

//take item from player
int playertakeall(std::string s){
	getitem(s)->quantity = 0;
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

	//print option text (this is no longer applicable)
	//clevertext(choosen->text, choosen->failed + choosen->used);

	clear();

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
				playertakeall(action.detail);
				break;
			case ATYPE_ENABLE:
				getoption(action.detail.substr(0, action.detail.find(":")),
					action.detail.substr(action.detail.find(":") + 1)
				)->enabled = true;
				break;
			case ATYPE_DISABLE:
				getoption(action.detail.substr(0, action.detail.find(":")),
					action.detail.substr(action.detail.find(":") + 1)
				)->enabled = false;
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
	clear();

	//check for endgame
	gameover();

	return false;
}

int main(){
	clear();
	ttext("Loading Data\n");
	ttext("...");

	//load the game data
	loadAllData();

	itext("\n\n[Done]");
	pause(500);
	clear();

	//start game loop
	bool fast = false;
	while(true){fast = loop(fast);}
	return 0;
}

//tracks data loader progress
struct Loader{
	int item_i;
	int room_i;
	int option_i;
} loader = {-1,-1,-1};

//these functions facilitate population "data" with game data
void newitem(std::string id, std::string text){
	Item a = {id,text,0};
	if(loader.item_i + 1 >= data.num_items){
		std::cout << "LOADER ERROR: no more space in item array\n";
		exit(EXIT_FAILURE);
	}
	data.items[++loader.item_i] = a;
}

void newroom(std::string id, int num_options){
	Room a = {id,"", (Option*) malloc(sizeof(Option)*num_options), num_options, 0};
	if(loader.room_i + 1 >= data.num_rooms){
		std::cout << "LOADER ERROR: no more space in room array\n";
		exit(EXIT_FAILURE);
	}

	data.rooms[++loader.room_i] = a;
	loader.option_i = -1;
}

void addtext(std::string s){
	if(loader.option_i == -1){
		data.rooms[loader.room_i].text += s;
	}else{
		data.rooms[loader.room_i].options[loader.option_i].text += s;
	}
}

void addoption(std::string id, bool enabled = true, bool single_use = false){
	Option a;
	a.id = id;
	a.text = "";
	a.passText = "";
	a.failText = "";
	a.used =0;
	a.failed = 0;
	a.enabled = enabled;
	a.disable_on_use = single_use;
	a.action = {ATYPE_NONE,"",NULL,false};
	a.condition = {CTYPE_NONE,""};
	a.displaytest = {CTYPE_NONE,""};

	if(loader.option_i + 1 >= data.rooms[loader.room_i].num_options){
		std::cout << "LOADER ERROR: no more space in option array [" + data.rooms[loader.room_i].id + "]\n";
		exit(EXIT_FAILURE);
	}
	data.rooms[loader.room_i].options[++loader.option_i] = a;
}

void require(std::string item){
    data.rooms[loader.room_i].options[loader.option_i].condition = {CTYPE_ITEM,item};
}

void passtext(std::string s){
	data.rooms[loader.room_i].options[loader.option_i].passText += s;
}

void failtext(std::string s){
	data.rooms[loader.room_i].options[loader.option_i].failText += s;
}

void disable(){
	data.rooms[loader.room_i].options[loader.option_i].enabled = false;
}

void single_use(){
	data.rooms[loader.room_i].options[loader.option_i].disable_on_use = true;
}


void action(int type, std::string detail){
	Action* b = (Action*) malloc(sizeof(Action));
	*b = {type,detail,NULL,false};

	Action* a = &(data.rooms[loader.room_i].options[loader.option_i].action);
	while(a->nextexists){
		a = a->next;
	}

	a->nextexists=true;
	a->next = b;
}

void copyoption(std::string id, std::string text=""){
	Option a = data.rooms[loader.room_i].options[loader.option_i];
	a.id = id; a.text = text;
	if(loader.option_i + 1 >= data.rooms[loader.room_i].num_options){
		std::cout << "LOADER ERROR: no more space in option array [" + data.rooms[loader.room_i].id + "]\n";
		exit(EXIT_FAILURE);
	}
	data.rooms[loader.room_i].options[++loader.option_i] = a;
}

//load game config into global variable data
void loadAllData(){
	//items
	#define ITEMCOUNT 7
	data.num_items = ITEMCOUNT;
	data.items = (Item*) malloc(sizeof(Item) *ITEMCOUNT);

	newitem("HF1", "a heart fragment");
	newitem("HF2", "a heart fragment");
	newitem("HF3", "a heart fragment");
	newitem("HF4", "a heart fragment");
	newitem("HF5", "a heart fragment");
	newitem("K1", "a rusty key");
	newitem("K1", "a clean key");

	//rooms
	/*
	newroom("",3);
	addtext("");

	addoption("1");
	addtext("");
	passtext("");
	action(ATYPE_MOVE, "");

	copyoption("2", "");
	copyoption("3", "");
	*/
	#define ROOMCOUNT 2
	data.num_rooms = ROOMCOUNT;
	data.rooms = (Room*) malloc(ROOMCOUNT*sizeof(Room));

	newroom("GMentry0",3);
	addtext("[Press Enter To Progress]~Hello>My name is Toby>Whats you name?");

	addoption("1");
	addtext("Itysbitta");
	passtext("!You Cannot Remember Your Name.");
	passtext("@~...~Hmm?>You cant speak?>I will name you then, let me think...~!.~..~...~@I know>I will call you: !shadow girl%!");
	action(ATYPE_MOVE, "LOCmainroom");

	copyoption("2", "Caxpwroana");
	copyoption("3", "Menalafani");

	newroom("LOCmainroom",1);
	addtext("You get a funny feeling.>Your vision goes white.>Whats going on?~...~|You materialise in the living room.");

	addoption("1");
	addtext("Do Nothing");
	passtext("!You did nothing. You lazy fuck.");

	//set entry point
	data.current = "GMentry0";
}

