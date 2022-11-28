#include <Windows.h>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <string>
#include <regex>
#include <thread>

using namespace std;

typedef struct Point {

	Point() = default;
	Point(bool p_is_life, int xt, int yt)
	{
		point_is_life = p_is_life;
		x = xt;
		y = yt;
	}
	bool point_is_life;
	int x = 0;
	int y = 0;
}point;

class Message {
public:
	void startMessage() {
		cout << "Hello in my fancy game of life!:)" << endl;
		cout << "There are two mods: offline and online" << endl;
		cout << "online mode: you just start programm and choose commands" << endl;
		cout << "offline mode: start it with -file <input file> -i <number of iterations> -o <output file>" << endl;
		cout << "There are such commands in online mode:" << endl;
		cout << "1. help - show game reference." << endl;
		cout << "2. dump <filename> -save current universe state." << endl;
		cout << "3. exit - end the game." << endl;
		cout << "4. tickn<n> -count game state after n iteration." << endl;
		Sleep(5000);
		system("cls");
	}
	void warningMessage() {
		cout << "Warning!" << endl;
		Sleep(500);
	}
	void unknowmcommand() {
		cout << "There is no such command!" << endl;
		Sleep(500);
	}
	void helpMessage() {
		cout << "There are such commands:" << endl;
		cout << "1. help - show game reference." << endl;
		cout << "2. dump<filename> -save current universe state." << endl;
		cout << "3. exit - end the game." << endl;
		cout << "4. tick<n> -count game state after n iteration." << endl;
		Sleep(5000);
	}

};

class CommandState
{
public:

	~CommandState() = default;

	int readCommand()
	{
		cin >> currentCommand;

		//Exit command
		if (currentCommand == "exit")
			return this->exit;

		//Dump game field command
		if (currentCommand == "dump")
			return this->dump;

		//Show field after n iteration
		if (currentCommand == "tick")
			return this->tick;

		//Show game reference
		if (currentCommand == "help")
			return this->help;

		return -1;
	}
	static int const exit = 1;
	static int const dump = 2;
	static int const tick = 3;
	static int const help = 4;
private:
	string currentCommand;
};

class GameState {
public:
	
	GameState(string file_name)
	{
		ifstream fin(file_name.c_str());
		regex universeNameRegex("[#N ]([A-Za-z]*)");
		regex roolsRegex("(#R )(B[0-9]+\/S[0-9]+)");
		regex sizeRegex("[#Size ]([0-9]+) ([0-9]+)");
		regex digits("[0-9]+");
		regex letters("[A-Za-z ]+");
		smatch s;

		char temp[256];

		//Read a name of universe
		fin.getline(temp, 256);
		if (std::regex_search(temp, universeNameRegex))
		{
			this->universeName = temp;
			this->universeName.erase(this->universeName.find("#N "), 3);
		}

		//Read rools for the game
		fin.getline(temp, 256);
		if (std::regex_search(temp, roolsRegex))
		{
			string str(temp);
			auto iter = sregex_iterator(str.begin(), str.end(), digits);
			s = *iter;
			this->gameRools = s.str();
			s = *(++iter);
			this->survivalRools = s.str();
		}

		//Read size of field
		fin.getline(temp, 256);
		if (std::regex_search(temp, sizeRegex))
		{
			std::string str(temp);
			auto iter = sregex_iterator(str.begin(), str.end(), digits);
			s = *iter;
			this->world_w = std::stol(s.str());
			s = *(++iter);
			this->world_h = std::stol(s.str());
		}

		//Read coordinates of cells
		int x, y;
		while (!fin.eof())
		{
			fin >> x >> y;
			this->cellsCoordinates.push_back(Point(true, x, y));
		}
		
	}
	string gameRools;
	string survivalRools;

	int world_h;
	int world_w;

	string fileName;
	string universeName;
	vector <point> cellsCoordinates;

};


class Field {

public:

	Field()= default;
	Field(GameState& _game)
	{
		game = &_game;
	}
	GameState* game;
	~Field() = default;
	vector<vector<point>> world;
	
	void init_world();
	void next_world();
	void print_world();
	void show_rools();
	void saveField(string outputFile);
private:
	//GameState* game;
	bool should_be_born(int x) {
		size_t found = game->gameRools.find(to_string(x));
		if (found != std::string::npos) return true;
		else return false;
	}
	bool should_survive(int x) {
		size_t found = game->survivalRools.find(to_string(x));
		if (found != std::string::npos) return true;
		else return false;
	}
};

void Field::show_rools()
{
	cout << "This universe have such Rules: " << endl;
	cout << "#N" << game->universeName << endl;
	cout << "#R " << "#B" << game->gameRools << "/" << "S" << game->survivalRools << endl;
	cout << "#S " << game->world_w << " " << game->world_h << endl;
	Sleep(1000);
}
void Field::saveField(string outputFile)
{
	std::ofstream fout(outputFile);
	fout << "#N " << game->universeName << endl;
	fout << "#R " << "#B" << game->gameRools << "/" << "S" << game->survivalRools << endl;
	fout << "#S " << game->world_w << " " << game->world_h << endl;

	for (size_t y = 0; y < game->world_h; y++)
	{
		for (size_t i = 0; i < game->world_w; i++)
		{
			if (world[i][y].point_is_life == true)
				fout << "* ";
			else
				fout << "  ";
		}
		fout << std::endl;
	}

}
void Field::next_world()
{
	vector<point> temp;
	
	int neighbours=0, _x, _y, x_, y_; //x and y on top and bottom(left and right for x axis) :)
	for (int y = 0; y < game->world_h; y++)
	{
		for (int i = 0; i < game->world_w; i++)
		{
			neighbours = 0;
			bool isAlive = world[i][y].point_is_life;
			if (y == 0 && i==0) {
				_x = game->world_w - 1;
				x_ = i + 1;
				_y = game->world_h - 1;
				y_ = y + 1;
			}
			else if (y==0 && i==game->world_w-1){
				_x = i - 1;
				x_ = 0;
				_y = game->world_h - 1;
				y_ = y + 1;
			}
			else if (y == game->world_h-1 && i == 0) {
				_x = game->world_w - 1;
				x_ = i + 1;
				_y = y-1;
				y_ = 0;
			}
			else if (y == game->world_h - 1 && i == game->world_w - 1) {
				_x = i - 1;
				x_ = 0;
				_y = y - 1;
				y_ = 0;
			}
			else if (y == 0) {
				_x = i - 1;
				x_ = i + 1;
				_y = game->world_h - 1;
				y_ = y + 1;
			}
			else if (y == game->world_h - 1) {
				_x = i - 1;
				x_ = i + 1;
				_y = y - 1;
				y_ = 0;
			}
			else if (i == 0) {
				_x = game->world_w - 1;
				x_ = i + 1;
				_y = y - 1;
				y_ = y + 1;
			}
			else if (i == game->world_w - 1) {
				_x = i - 1;
				x_ = 0;
				_y = game->world_h - 1;
				y_ = y + 1;
			}
			else {
				_x = i - 1;
				x_ = i + 1;
				_y = y - 1;
				y_ = y + 1;
			}
			if (world[_x][_y].point_is_life == true) neighbours++;
			if (world[i][_y].point_is_life == true) neighbours++;
			if (world[x_][_y].point_is_life == true) neighbours++;

			if (world[_x][y].point_is_life == true) neighbours++;
			if (world[x_][y].point_is_life == true) neighbours++;

			if (world[_x][y_].point_is_life == true) neighbours++;
			if (world[i][y_].point_is_life == true) neighbours++;
			if (world[x_][y_].point_is_life == true) neighbours++;
			
			if (should_be_born(neighbours) && !isAlive) 
				temp.push_back(Point{ true, i, y });

			if (should_survive(neighbours) && isAlive) 
				temp.push_back(Point{ true, i, y });

			if (!should_survive(neighbours) && isAlive) 
				temp.push_back(Point{ false, i, y });

		}
	}
	for (auto& x : temp)
	{
		if (x.point_is_life) this->world[x.x][x.y].point_is_life = true;
		else this->world[x.x][x.y].point_is_life = false;
	}
	temp.resize(0);
}

void Field::init_world()
{
	vector<point> temp;
	world.reserve(game->world_h);
	for (size_t y = 0; y < game->world_h; y++)
	{
		temp.resize(0);
		for (size_t i = 0; i < game->world_w; i++)
		{
			point z ={ 0, 0, 0 };
			temp.push_back(z);
		}
		world.push_back(temp);
	}

	for (auto &x : this->game->cellsCoordinates)
	{
		temp.reserve(game->world_w);
		world[x.x][x.y] = { true, x.x, x.y };
	}
	
}


void Field::print_world()
{
	for (int y = 0; y < game->world_h; y++) {
		for (int x = 0; x < game->world_w; x++) {
			if (world[x][y].point_is_life == true) {
				cout << "* ";
			}
			else
				cout << "  ";
		}
		cout << endl;
	}
}

int main(int argc, char** argv) {
	int commandArg = 0, iterations = 0, gameMode = 1;
	string inputFile, outputFile;
	Message mym;
	mym.startMessage();
	
	if (argc > 1) {
		for (int i = 1; i < argc; i++)
		{
			gameMode = 0;
			if (argv[i] == string("-i"))
			{
				iterations = stoi(argv[i + 1]);
			}

			if (argv[i] == string("-o"))
			{
				string temp = argv[i + 1];
				outputFile = string(temp.begin(), temp.end());
			}
			if (argv[i] == string("-file"))
			{
				string temp = argv[i + 1];
				inputFile = string(temp.begin(), temp.end());
			}
		}
	}

	string file;
	if (inputFile.empty())
		file="4so.txt";
	else
		file=inputFile;

	GameState mylife(file);
	Field field(mylife);
	field.init_world();
	field.show_rools();
	int command;
	if (gameMode == 1) {
		CommandState com;
		
		while (true) {
			command = com.readCommand();
			if (command == com.dump) {
				field.saveField(outputFile);
				cout << "file saved in: " << outputFile << "!";
			}
			else if (command == com.exit) {
				cout << "Thank you for playing, hope to see you again :)" << endl;
				exit(0);
			}
			else if (command == com.help) {
				mym.helpMessage();
				
			}
			else if (command == com.tick) {
				cin >> iterations;
				while (iterations > 0) {
					system("cls");
					field.print_world();
					Sleep(100);
					field.next_world();
					iterations--;
					
				}

			}
			else if (command == -1) {
				mym.warningMessage();
				mym.unknowmcommand();
				
			}
			
		}
	}
	else {
		while (iterations > 0) {
			system("cls");
			field.print_world();
			Sleep(100);
			field.next_world();
			iterations--;
		
			
		}
		field.saveField(outputFile);
		cout << "file saved in: " << outputFile << "!";
	}

	return 0;
}