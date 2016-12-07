//Author: Erich Spaker
//Class: ECE 114
//Project: final game project
//Game Name: Shadow Girl's Text Adventure
//Description: This is a text based adventure game.
//	Press enter to progress
//	Enter number to select option
//	Most of my time went toward the engine
//	So the story is fairly short. Sorry.
//	The demo story shows the engines main features:
//	dialog, options, rooms, and items
// 	The code is devided into three parts.
//	DATA: the structures at the top of the file define how data for the game is stored
//		rooms have header text and options to choose from
//		options have conditions, text to print after passing or failing that condition
//			and actions to perform if they pass the condition
//		actions can be changing rooms, giving items, or enabling/disabling options
//		the only implemented condition is an item requirement.
//	ENGINE: Below the structs is the engine code which interprits and manipulates the game data structs
//		in order to put text on the screen. This code consists of helper functions and the function loop
//	LOADER: These functions come after main and facilitate populating the game data structs with actual game data
//		The last function uses these helper functions to create the game.
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

Item NullItem = {" ", "Not everything exists.", 0};
Room NullRoom = {" ", "Not everywhere exists.", NULL, 0};
Option NullOption = {" ", "Some things aren't a choice."};

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
std::string ascii_heart = " ________________ \n|      _  _      |\n|   \\ ( \\/ ) /   |\n|  --  \\  \n";
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

void flushcin(){
	std::cin.sync();
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
	ttext("\nDear Programmer: " + s + "\n");
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
	if(getroom(s)->id != " "){
		data.current = s;
		return true;
	}

	return false;
}

//check if player has all heart fragments
void gameover(){
	if(playerhas("HF1") &&
		playerhas("HF2") &&
		playerhas("HF3") &&
		playerhas("HF4") &&
		playerhas("HF5")
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
		flushcin();
		while(true) ttextp("CLOSE TERMINAL TO EXIT");
	}
}

int loop(bool fast){
	//display room text.
	std::string roomid = data.current;
	Room* current = getroom(roomid);
	if(fast){
		clevertext(CLEVER_SKIP+current->text,current->passes>0);
	}else{
		clevertext(current->text,current->passes>0);
	}

	//display options.
	int options[100];
	int num_options=0;
	std::cout<<std::endl;
	for(int i = 0 ; i < current->num_options ; i++){
		bool spent = current->options[i].used > 0 && current->options[i].disable_on_use;
		if(current->options[i].enabled && !spent){//todo display condition
			itext(" " + std::to_string(num_options+1) + " " +  current->options[i].text + endl);
			pause(TTEXT_RATE*4);
			options[num_options]=i;
			num_options++;
		}
	}
	(current->passes)++;

	//get choice.
	int select = 0;
	while(true){
		flushcin();
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
		(choosen->failed)++;
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

	(choosen->used)++;

	//check to make sure everything is okay
	if(getroom(roomid)->options[select].used == 0){
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
} loader;

//these functions facilitate population "data" with game data
void newitem(std::string id, std::string text){
	if(loader.item_i + 1 >= data.num_items){
		std::cout << "LOADER ERROR: no more space in item array\n";
		exit(EXIT_FAILURE);
	}

	loader.item_i++;
	data.items[loader.item_i].id = id;
	data.items[loader.item_i].text = text;
	data.items[loader.item_i].quantity = 0;
}

void newroom(std::string id, int num_options){
	if(loader.room_i + 1 >= data.num_rooms){
		std::cout << "LOADER ERROR: no more space in room array\n";
		exit(EXIT_FAILURE);
	}
	Option* options = new Option[num_options];

	loader.room_i++;
	data.rooms[loader.room_i].id = id;
	data.rooms[loader.room_i].options = options;
	data.rooms[loader.room_i].num_options = num_options;
	data.rooms[loader.room_i].text = " ";
	data.rooms[loader.room_i].passes = 0;

	loader.option_i = -1;
}

void addtext(std::string s){
	if(loader.option_i == -1){
		if(data.rooms[loader.room_i].text != " "){
			data.rooms[loader.room_i].text += s;
		}else{
			data.rooms[loader.room_i].text = s;
		}
	}else{
		if(data.rooms[loader.room_i].options[loader.option_i].text != " "){
			data.rooms[loader.room_i].options[loader.option_i].text += s;
		}else{
			data.rooms[loader.room_i].options[loader.option_i].text = s;
		}
	}
}

void addoption(std::string id, bool enabled = true, bool single_use = false){
	Option a;
	a.id = id;
	a.text = " ";
	a.passText = " ";
	a.failText = " ";
	a.used =0;
	a.failed = 0;
	a.enabled = enabled;
	a.disable_on_use = single_use;
	a.action = {ATYPE_NONE," ",NULL,false};
	a.condition = {CTYPE_NONE," "};
	a.displaytest = {CTYPE_NONE," "};

	if(loader.option_i + 1 >= data.rooms[loader.room_i].num_options){
		std::cout << "LOADER ERROR: no more space in option array [" + data.rooms[loader.room_i].id + "]\n";
		exit(EXIT_FAILURE);
	}

	++(loader.option_i);
	data.rooms[loader.room_i].options[loader.option_i] = a;
}

void require(std::string item){
    data.rooms[loader.room_i].options[loader.option_i].condition = {CTYPE_ITEM,item};
}

void passtext(std::string s){
	if(	data.rooms[loader.room_i].options[loader.option_i].passText != " "){
		data.rooms[loader.room_i].options[loader.option_i].passText += s;
	}else{
		data.rooms[loader.room_i].options[loader.option_i].passText = s;
	}
}

void failtext(std::string s){
	if(	data.rooms[loader.room_i].options[loader.option_i].failText != " "){
		data.rooms[loader.room_i].options[loader.option_i].failText += s;
	}else{
		data.rooms[loader.room_i].options[loader.option_i].failText = s;
	}
}

void disable(){
	data.rooms[loader.room_i].options[loader.option_i].enabled = false;
}

void single_use(){
	data.rooms[loader.room_i].options[loader.option_i].disable_on_use = true;
}


void action(int type, std::string detail){
	Action* a = &(data.rooms[loader.room_i].options[loader.option_i].action);

	while(a->nextexists){
		a = a->next;
	}

	a->nextexists=true;

	a->next = new Action;
	a->next->type = type;
	a->next->detail = detail;
	a->next->next = NULL;
	a->next->nextexists = false;
}

void copyoption(std::string id, std::string text=" "){
	Option a = data.rooms[loader.room_i].options[loader.option_i];
	a.id = id; a.text = text;
	if(loader.option_i + 1 >= data.rooms[loader.room_i].num_options){
		std::cout << "LOADER ERROR: no more space in option array [" + data.rooms[loader.room_i].id + "]\n";
		exit(EXIT_FAILURE);
	}
	data.rooms[loader.room_i].options[++(loader.option_i)] = a;
}

//load game config into global variable data
void loadAllData(){
	loader={-1,-1,-1};

	//items
	#define ITEMCOUNT 7
	data.num_items = ITEMCOUNT;
	data.items = new Item[ITEMCOUNT];

	newitem("HF1", "a heart fragment");
	newitem("HF2", "a heart fragment");
	newitem("HF3", "a heart fragment");
	newitem("HF4", "a heart fragment");
	newitem("HF5", "a heart fragment");
	newitem("K1", "a rusty key");
	newitem("K2", "a clean key");
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
	#define ROOMCOUNT 5
	data.num_rooms = ROOMCOUNT;
	data.rooms = new Room[ROOMCOUNT];

	newroom("GMentry0",3);
	addtext("[Press Enter To Progress]~You get a funny feeling.>Something isn't right.~...~You come to a realization.>Where is your body??~[CONTINUE]~|You materialize in the living room.");
	addoption("1");
		addtext("Talk to the child.");
		passtext("@Child: Hey Ghost Person...~!You approach the child.");
		action(ATYPE_MOVE, "GMentry1");
		disable();

		addoption("2");
		addtext("Try to remember.");
		passtext("Last you remember you were climbing a tree.~|You have to find your body...>soon.");

		addoption("3");
		addtext("Look around.");
		passtext("It's a fairly normal living room.>It kind of reminds you of yours.~The furniture is quite differn't though.>|Come to think of it there are a lot of strange things in this room.~!You notice a kid in the corner.");
		action(ATYPE_ENABLE, "GMentry0:1");

	newroom("GMentry1",3);
	addtext("Child: Hey>Child: You're not a ghost.>Child: (Just another dumb shadow)>Child: I never find any ghosts.\nThis place is supposed to be haunted\nbut it always turns out to be shadows.~Child: You seem confused>Child: Sorry, I didn't mean to be rude\nMy Name is Toby.>|Child: What's your name.");
		addoption("1");
		addtext("Itysbitta");
		passtext("You: fasdfgkjabfdgmdasfjhg~");
		passtext("@...~Toby: Hmm?>Toby: You cant speak?>Toby: I'll just call you shadowgirl for now.");
		action(ATYPE_MOVE, "LOCmainroom");

		copyoption("2", "Caxpwroana");
		copyoption("3", "Menalafani");

	newroom("LOCmainroom",3);
	addtext("You are in the living room.");
		addoption("1");
		addtext("Enter the basement.");
		passtext("The light of you heart casts your shadow.>|You descend into the darkness.>Toby follows.");
		failtext("A shadow cannot exist without light>|It's too dark to progress.");
		action(ATYPE_MOVE,"LOCbasement");
		require("HF1");

		addoption("2");
		addtext("Go to the kitchen.");
		passtext("[Continue]");
		action(ATYPE_MOVE, "LOCkitchen");

		addoption("3");
		addtext("Talk to Toby.");
		passtext("Toby: The basement is dark and scary.>Toby: I'm afraid to go down there.>Toby: I BET THERES A GHOST DOWN THERE TOO%!%!%!>You: %<nod%>>|Toby: If only we had a light.");

	newroom("LOCkitchen",4);
	addtext("The kitchen is a mess.>Its unlikely anyone has lived here in a long time.>There is a black cat looking at you curiously.>There is something shiny on top of the cabinets~[Continue]~|You are in the kitchen.");
		addoption("1");
		addtext("Reach for the shiny thing.");
		passtext("@Your not sure if you can even pick it up without your body.>It doesn't matter though.>|You can't reach.");

		addoption("2");
		addtext("Approach the cat.");
		passtext("Cat: meOWW.~The cat is spooked by your movement.>It's almost like he's never seen a disembodied shadow before.>He leaps away from you onto the cabinet.>The shiny thing is knocked onto the floor.");
		action(ATYPE_ENABLE,"LOCkitchen:3");
		action(ATYPE_DISABLE,"LOCkitchen:1");
		single_use();

		addoption("3");
		addtext("Pick up the shiny thing.");
		passtext("You are strangely drawn to the glowing red crystal,\nand as you near it, the glow intensifies.>You get the feeling...>This crystal...>is a part of you.~You reach for it.~It dissolves into your chest.~A portion of your heart has been restored.>You can feel the light in you veins.~!Achievement Unlocked: [you can now enter dark places]");
		action(ATYPE_ITEM, "HF1");
		disable();
		single_use();

		addoption("4");
		addtext("Return to livingroom.");
		passtext("[Continue]");
		action(ATYPE_MOVE, "LOCmainroom");

	newroom("LOCbasement",2);
	addtext("[Continue]~This is all I had time to program.>Most work went toward the engine.>You can now skip to the end or go back to the beginning.>I hope you enjoyed this demo.~|@You are in the basement.");
		addoption("1");
		addtext("Skip to end.");
		passtext("[Continue]");
		action(ATYPE_ITEM, "HF2");
		action(ATYPE_ITEM, "HF3");
		action(ATYPE_ITEM, "HF4");
		action(ATYPE_ITEM, "HF5");


		addoption("2");
		addtext("Return upstairs.");
		passtext("[Continue]");
		action(ATYPE_MOVE, "LOCmainroom");

	//set entry point
	data.current = "GMentry0";
}

