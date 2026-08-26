#include<io.h>
#include<regex>
#include<cctype>
#include<vector>
#include<conio.h>
#include<iostream>
#include<algorithm>
#include<windows.h>
using std::cin;
using std::cout;
using std::endl;
using std::regex;
using std::string;
using std::vector;
string cd="";
int items_per_page=28;
int chars_per_line=120;
int start_index=0,active_index=0;
int num_results;
vector<string> search_results;
typedef enum
{
	TYPE_FILE=0,
	TYPE_DIRECTORY=1,
	TYPE_CURRENT_DIRECTORY=2,
	TYPE_ERROR=3,
} item_type;
int color_mapper[]={0x07,0x0a,0x0d,0x0c};
int normal_color=0x07;
int select_mask=0x48;
int hidden_mask=0x90;
void reset_cursor()
{
	HANDLE hdout=GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos={0,0};
	SetConsoleCursorPosition(hdout,pos);
}
void move_cursor(short x,short y)
{
	HANDLE hdout=GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos={x,y};
	SetConsoleCursorPosition(hdout,pos);
}
inline void warning()
{
	MessageBeep(MB_ICONEXCLAMATION);
}
inline void error()
{
	MessageBeep(MB_ICONERROR);
}
string list_available_disks()
{
	string available_drive_list="";
	for(char c='A';c<'Z';c++)
		if(_access((c+string(":\\")).c_str(),0)!=-1)
			available_drive_list+=c;
	return available_drive_list;
}
void output_item(string s,item_type it,bool selected,bool hidden,int max_len,bool newline=true)
{
	if(int(s.length())>=max_len)
	{
		if(it==TYPE_CURRENT_DIRECTORY)
			s="..."+s.substr(s.length()-max_len+4);
		else
			s=s.substr(0,max_len-4)+"...";
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),color_mapper[it]|(selected?select_mask:0)|(hidden?hidden_mask:0));
	if(newline)
		cout<<s<<endl;
	else
		cout<<s;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),normal_color);
}
int count_dir(string dir)
{
	if(dir.rfind("<search",0)==0)
		return num_results;
	int count=0;
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	hFind=FindFirstFile(("\\\\?\\"+dir+"*.*").c_str(),&findData);
	do
	{
		if(strcmp(findData.cFileName,".")!=0&&strcmp(findData.cFileName,"..")!=0)
			count++;
	}
	while(FindNextFile(hFind,&findData));
	FindClose(hFind);
	return count;
}
void list_dir(string dir,int start_index,int lines)
{
	if(dir.rfind("<search",0)==0)
	{
		output_item(dir,TYPE_CURRENT_DIRECTORY,false,false,chars_per_line); 
		if(dir[9]=='o')//<search folder>
			for(int i=start_index;i<start_index+lines;i++)
			{
				if(i<num_results)
					output_item(search_results[i],TYPE_DIRECTORY,active_index==i,GetFileAttributes(("\\\\?\\"+search_results[i]).c_str())&FILE_ATTRIBUTE_HIDDEN,chars_per_line);	
			}
		else//<search file>
			for(int i=start_index;i<start_index+lines;i++)
			{
				if(i<num_results)
					output_item(search_results[i],TYPE_FILE,active_index==i,GetFileAttributes(("\\\\?\\"+search_results[i]).c_str())&FILE_ATTRIBUTE_HIDDEN,chars_per_line);	
			}
		return;
	}
	int count=0;
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	output_item(dir,TYPE_CURRENT_DIRECTORY,false,false,chars_per_line); 
	hFind=FindFirstFile(("\\\\?\\"+dir+"*.*").c_str(),&findData);
	if(hFind==INVALID_HANDLE_VALUE)
	{
		output_item("[unable to access]",TYPE_ERROR,false,false,chars_per_line);
		error();
		return;
	}
	do
	{
		if(strcmp(findData.cFileName,".")!=0&&strcmp(findData.cFileName,"..")!=0)
			count++;
		if(count>start_index&&count<=start_index+lines)
		{
			if((findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0&&strcmp(findData.cFileName,".")!=0&&strcmp(findData.cFileName,"..")!=0)
				output_item(findData.cFileName,TYPE_DIRECTORY,(active_index==(count-1)),findData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN,chars_per_line);
			else if(strcmp(findData.cFileName,".")!=0&&strcmp(findData.cFileName,"..")!=0)
				output_item(findData.cFileName,TYPE_FILE,(active_index==(count-1)),findData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN,chars_per_line);
		}
	}
	while(FindNextFile(hFind,&findData));
	FindClose(hFind);
}
string get_item_name(string dir,int index)
{
	if(dir.rfind("<search",0)==0)
	{
		if(index<num_results)
			return search_results[index];
		else
			return "";
	}
	int count=0;
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	hFind=FindFirstFile(("\\\\?\\"+dir+"*.*").c_str(),&findData);
	do
	{
		if(strcmp(findData.cFileName,".")!=0&&strcmp(findData.cFileName,"..")!=0)
			count++;
		if(count-1==index)
		{
			FindClose(hFind);
			return findData.cFileName;	
		}
	}
	while(FindNextFile(hFind,&findData));
	FindClose(hFind);
	return "";
}
string parent_directory(string directory)
{
	if(directory.rfind("<search",0)==0)
	{
		if(directory[9]=='o')//<search folder>
			return directory.substr(15);
		else//<search file>
			return directory.substr(13);
	}
	size_t n;
	if(directory[directory.length()-1]=='\\')
		n=directory.substr(0,directory.length()-1).rfind('\\');
	else
		n=directory.rfind('\\');
	if(n!=string::npos)
		return directory.substr(0,n+1);
	else
		return "";
}
string name(string path)
{
	int len=path.length();
	if(len==3&&path[1]==':'&&path[2]=='\\')
		return path.substr(0,len-1);
	else
	{
		if(path[len-1]=='\\')
		{
			int begin=path.rfind("\\",len-2)+1;
			return path.substr(begin,len-begin-1);
		}
		else
		{
			int begin=path.rfind("\\")+1;
			return path.substr(begin);
		}
	}
}
void open_item(string path,string start_path)
{
	DWORD attr=GetFileAttributes(("\\\\?\\"+path).c_str());
	DWORD last_error=GetLastError();
	if(attr!=INVALID_FILE_ATTRIBUTES)
		last_error=ERROR_SUCCESS;
	if(last_error!=ERROR_SUCCESS)
	{
		error();
		move_cursor(0,items_per_page+1);
		cout<<"unable to access \""<<path<<"\":error "<<last_error;
		while(!_kbhit());
		return;
	}
	if(attr&FILE_ATTRIBUTE_DIRECTORY)
	{
		active_index=0;
		start_index=0;
		if(path[path.length()-1]=='\\')
			cd=path;
		else
			cd=path+"\\";
	}
	else
		ShellExecute(nullptr,"open",path.c_str(),nullptr,start_path.c_str(),SW_SHOW);
}
void search_dir(string dir,const regex& rx,bool match_folder,vector<string>& results)
{
	if(dir.rfind("<search",0)==0)
	{
		for(int i=0;i<num_results;i++)
		{
			if(search_results[i][search_results[i].length()-1]=='\\')//is a directory
			{
				if(match_folder)
				{
					if(regex_match(name(search_results[i]),rx))
						results.push_back(search_results[i]);
				}
				else
					search_dir(search_results[i].substr(0,search_results[i].length()-1),rx,false,results);
			}
			else//is a file
			{
				if(match_folder)
				{
					if(regex_match(name(parent_directory(search_results[i])),rx))
						results.push_back(search_results[i]);
				}
				else
				{
					if(regex_match(name(search_results[i]),rx))
						results.push_back(search_results[i]);
				}
			}
		}
	}
	else
	{
		string pattern;
		if(dir.find("\\\\?\\")!=0)
			pattern="\\\\?\\"+dir+"\\*.*";
		else
			pattern=dir+"\\*.*";
		WIN32_FIND_DATA fd;
		HANDLE hFind=FindFirstFile(pattern.c_str(),&fd);
		if(hFind==INVALID_HANDLE_VALUE)
			return;
		do
		{
			if(strcmp(fd.cFileName,".")==0||strcmp(fd.cFileName,"..")==0)
				continue;
			if(fd.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)//skip reparse points(symbolic links, etc.) to avoid infinite recursions
				continue;
			string full=dir+"\\"+fd.cFileName;
			bool is_dir=(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
			if(is_dir)
			{
				search_dir(full,rx,match_folder,results);
				if(match_folder&&regex_match(fd.cFileName,rx))
					results.push_back(full+"\\");
			}
			else
			{
				if(!match_folder&&regex_match(fd.cFileName,rx))
					results.push_back(full);
			}
		}
		while(FindNextFile(hFind,&fd));
		FindClose(hFind);
	}
}
//only matches absolute paths, . or .. allowed
inline bool is_path_valid(string p)
{
	static regex path_pattern(R"([A-Z]:\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]*)");
	return regex_match(p,path_pattern);
}
//does not check if dir is valid
void clone_process(string dir)
{
	string path;
	path.resize(32767*3);//at most 32767 wide chars, 1 wide char=at most 3 bytes
	path.resize(GetModuleFileNameA(nullptr,&path[0],static_cast<DWORD>(path.size())));//basically reads the path and finishes resizing at the same time
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si,0,sizeof(si));
	si.cb=sizeof(si);
	memset(&pi,0,sizeof(pi));
	string cmd="\""+path+"\" \""+dir+"\\\"";//extra \ is important, it stops Windows from escaping the string
	//Windows potentially writes commandline during parsing, so dir needs to be writable
	CreateProcess(path.c_str(),&cmd[0],nullptr,nullptr,false,CREATE_NEW_CONSOLE,nullptr,nullptr,&si,&pi);
}
void clrscr()
{
	HANDLE hdout=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout,&csbi);
	DWORD size=csbi.dwSize.X*(items_per_page+2),num=0;
	COORD pos={0,0};
	SetConsoleTextAttribute(hdout,normal_color);
	FillConsoleOutputCharacter(hdout,' ',size,pos,&num);
	FillConsoleOutputAttribute(hdout,normal_color,size,pos,&num);
	SetConsoleCursorPosition(hdout,pos);
}
void clear_line(short line)
{
	if(line<0)
		return;
	HANDLE hdout=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout,&csbi);
	DWORD size=csbi.dwSize.X;
	string s(size,' ');
	SetConsoleCursorPosition(hdout,{0,line});
	cout<<s;
	SetConsoleCursorPosition(hdout,{0,line});
}
void handle_operation()
{
	int c1=_getch(),c2;
	switch(c1)
	{
		case 13://enter
			if(cd.rfind("<search",0)==0)
				open_item(get_item_name(cd,active_index),parent_directory(get_item_name(cd,active_index)));
			else
				open_item(cd+get_item_name(cd,active_index),cd);
			clrscr();
			break;
		case 14://ctrl-n(new window)
			if(cd.rfind("<search",0)==0)
				clone_process(parent_directory(cd));
			else
				clone_process(cd);
			break;
		case 18://ctrl-r(refresh)
			clrscr();
			break;
		case 27://esc
			cd=parent_directory(cd);
			start_index=0;
			active_index=0;
			clrscr();
			break;
		case 101://e(open in explorer)
		{
			string target=cd;
			if(target.rfind("<search",0)==0)
				target=parent_directory(cd);
			ShellExecute(nullptr,"open",target.c_str(),nullptr,nullptr,SW_SHOW);
			clrscr();
			break;
		}
		case 104://h(home, go to root directory)
			cd="";
			clrscr();
			break;
		case 106://j(jump to)
		{
			clrscr();
			list_dir(cd,start_index,items_per_page);
			move_cursor(0,items_per_page+1);
			cout<<"jump to:";
			string s;
			getline(cin,s);
			if(s=="")
			{
				clrscr();
				break;
			}
			if(!is_path_valid(s))
			{
				clrscr();
				list_dir(cd,start_index,items_per_page);
				move_cursor(0,items_per_page+1);
				output_item("invalid path \""+s+"\"",TYPE_ERROR,false,false,chars_per_line,false);
				while(!_kbhit());
				clrscr();
				break;
			}
			DWORD attr=GetFileAttributes(("\\\\?\\"+s).c_str());
			if(attr==INVALID_FILE_ATTRIBUTES)
			{
				clrscr();
				list_dir(cd,start_index,items_per_page);
				move_cursor(0,items_per_page+1);
				output_item("inaccessible path \""+s+"\"",TYPE_ERROR,false,false,chars_per_line,false);
				while(!_kbhit());
				clrscr();
				break;
			}
			//paths containing only '.'s for folder names not allowed, like A:\a\...\b
			regex danger_pattern(R"(\\\.+(?:\\|$))");
			if(regex_search(s,danger_pattern))
			{
				clrscr();
				list_dir(cd,start_index,items_per_page);
				move_cursor(0,items_per_page+1);
				output_item("path contains only \".\"s for folder name",TYPE_ERROR,false,false,chars_per_line,false);
				while(!_kbhit());
				clrscr();
				break;
			}
			if(!(attr&FILE_ATTRIBUTE_DIRECTORY))
			{
				clrscr();
				list_dir(cd,start_index,items_per_page);
				move_cursor(0,items_per_page+1);
				output_item("cannot jump to file \""+s+"\"",TYPE_ERROR,false,false,chars_per_line,false);
				while(!_kbhit());
				clrscr();
				break;
			}
			cd=s;
			if(cd[cd.length()-1]!='\\')
				cd=cd+"\\";
			start_index=0;
			active_index=0;
			clrscr();
			break;
		}
		case 114://r(rename)
		{
			string s;
			do
			{
				clear_line(active_index-start_index+1);
				getline(cin,s);
				if(s=="")
					break;
				if(MoveFile(("\\\\?\\"+cd+get_item_name(cd,active_index)).c_str(),("\\\\?\\"+cd+s).c_str()))
					break;
				else
					warning();
			}
			while(1);
			clrscr();
			break;
		}
		case 115://s(search)
		{
			string s;
			move_cursor(0,items_per_page+1);
			cout<<"search:";
			getline(cin,s);
			clrscr();
			list_dir(cd,start_index,items_per_page);//refresh the TUI to make sure it doesn't scroll due to the ENTER after search input
			if(s=="")
				break;
			regex pattern(R"((file|folder) .*)");
			if(!regex_match(s,pattern))
			{
				error();
				move_cursor(0,items_per_page+1);
				cout<<"invalid search string \""<<s<<"\"";
				while(!_kbhit());
				clrscr();
				break;
			}
			vector<string> result;
			bool folder=s[1]=='o';//if the rule starts with folder, then this is true, otherwise false
			string rule=s.substr(folder?7:5);
			try
			{
				regex search_pattern(rule);
				clrscr();
				list_dir(cd,start_index,items_per_page);//refresh the TUI to make sure it doesn't scroll due to the ENTER after search input
				move_cursor(0,items_per_page+1);
				cout<<"searching...";
				search_dir(cd.substr(0,cd.length()-1),search_pattern,folder,result);//remove the trailing "\" to prevent results from having extra "\"s
				if(cd.rfind("<search",0)==0)
				{
					if(cd[9]=='o')//folder
						cd=(folder?"<search folder>":"<search file>")+cd.substr(15);
					else//file
						cd=(folder?"<search folder>":"<search file>")+cd.substr(13);
				}
				else
					cd=(folder?"<search folder>":"<search file>")+cd;
				search_results=result;
				sort(search_results.begin(),search_results.end());
				search_results.erase(unique(search_results.begin(),search_results.end()),search_results.end());
				num_results=search_results.size();
				start_index=0;
				active_index=0;
				clrscr();
				break;
			}
			catch(...)//only possible error is regex_error, no need to catch it
			{
				error();
				move_cursor(0,items_per_page+1);
				cout<<"invalid regular expression \""<<rule<<"\"";
				while(!_kbhit());
				clrscr();
				break;
			}
		}
		case 224://up,down,left,right,pageup,pagedown
			c2=_getch();
			switch(c2)
			{
				case 73://pageup
					if(start_index>=items_per_page)
					{
						start_index-=items_per_page;
						active_index=start_index;
						clrscr();
					}
					else
					{
						warning();
						reset_cursor();
					}
					break;
				case 81://pagedown
					if(start_index+items_per_page<=count_dir(cd))
					{
						start_index+=items_per_page;
						active_index=start_index;
						clrscr();
					}
					else
					{
						warning();
						reset_cursor();
					}
					break;
				case 72:case 75://up,left
					if(active_index==start_index)
					{
						if(start_index>=items_per_page)
						{
							start_index-=items_per_page;
							active_index--;
							clrscr();
						}
						else
						{
							warning();
							reset_cursor();
						}
					}
					else
					{
						active_index--;
						reset_cursor();
					}
					break;
				case 80:case 77://down,right
					if(active_index==start_index+items_per_page-1)
					{
						if(start_index+items_per_page<count_dir(cd))
						{
							start_index+=items_per_page;
							active_index=start_index;
							clrscr();
						}
						else
						{
							warning();
							reset_cursor();
						}
					}
					else if(active_index<count_dir(cd)-1)
					{
						active_index++;
						reset_cursor();
					}
					else
					{
						warning();
						reset_cursor();
					}
					break;
//delete function is removed because of strange behavior (potentially damaging files)
//				case 83://delete
//					delete_item(cd+get_item_name(cd,active_index));
//					if(active_index>=count_dir(cd))
//					{
//						active_index=count_dir(cd)-1;
//						if(start_index>=items_per_page)
//						{
//							start_index-=items_per_page;
//							active_index=start_index;
//							clrscr();
//						}
//						else
//							reset_cursor();
//					}
//					clrscr();
//					break;
				default:
					warning();
					clrscr();
					break;
			}
			break;
		default:
			warning();
			clrscr(); 
			break;
	}
}
void select_disk()
{
	string s=list_available_disks();
	char c;
	do
	{
		reset_cursor();
		cout<<"available disks:";
		for(unsigned long long i=0;i<s.length();i++)
			cout<<s[i]<<" ";
		cout<<"\nselect a disk:";
		c=_getch();
	}
	while(c<0||(s.find(c)==string::npos&&s.find(toupper(c))==string::npos));
	cd=char(toupper(c))+string(":\\");
}
int main(int argc,char** argv)
{
	if(argc>1&&is_path_valid(argv[1]))
		cd=argv[1];
	while(1)
	{
		if(cd!="")
			list_dir(cd,start_index,items_per_page);
		else
		{
			select_disk();
			clrscr();
			start_index=0;
			active_index=0;
			list_dir(cd,start_index,items_per_page);
		}
		handle_operation();
	}
}
