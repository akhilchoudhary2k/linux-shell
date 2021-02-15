


// Operating Systems Lab Assignment


#include <bits/stdc++.h>
#include <unistd.h> 		// to include exec family functions
#include <sys/wait.h> 		// to include wait function
#include <fcntl.h>		    // to include open function
using namespace std;


#define ANSI_RED "\033[0;31m"
#define ANSI_GREEN "\033[0;32m"
#define ANSI_YELLOW "\033[0;33m"
#define ANSI_CYAN "\033[0;36m"
#define ANSI_RED_BOLD "\033[1;31m"
#define ANSI_GREEN_BOLD "\033[1;32m"
#define ANSI_YELLOW_BOLD "\033[1;33m"
#define ANSI_CYAN_BOLD "\033[1;36m"
#define ANSI_PURPLE "\033[0;35m"
#define ANSI_BLUE "\033[0;34m"
#define ANSI_BLUE_BOLD "\033[1;34m"
#define ANSI_DEFAULT "\033[0m"
#define ANSI_CLEAR "\033[2J\033[1;1H"




// Function declarations ======================================================

// removes the spaces from front and end of string
string trim_outer_spaces(const string &s);

// splits a string into substrings based on a delimiter
vector<string> split(const string &s, char delimiter);

// will set the input redirections if s is not empty
void set_input_redirection(string& s);

// will set the output redirections if s is not empty
void set_output_redirection(string& s);

// will take a command like    a < b > c  and split the strings a,b,c and return them in ans vector
vector<string> split_by_redirector(string cmd);

// will take a command as a string and execute it using execvp syscall
void execute_command(string command);

void display_help(){
	printf(ANSI_CLEAR);

	printf(ANSI_RED_BOLD);
	printf("\n\t*******************************************\n");
	printf("\t*                                         *\n");
	printf("\t*\tWelcome to my Linux-Shell    \t  *\n");
	printf("\t*                                         *\n");
	printf("\t*                                         *\n");
	printf("\t*\tTye 'exit' to leave this shell.\t  *\n");
	printf("\t*                                         *\n");
	printf("\t*******************************************\n\n");
	printf(ANSI_DEFAULT);
}


// will return 1 when it needs to be called again
// otherwise 0
int shell_loop(){
	
	while(1){

		string command;
		bool is_background = false;
			
	    // get current working directory
	    char cwd[1000];
	    getcwd(cwd, sizeof(cwd)); 
	    char *username;
	    username = getenv("USER");
	    char hostname[1024];
  		gethostname(hostname, 1025);

	    
	    printf(ANSI_GREEN_BOLD);
	    printf("%s@%s : ", getenv("USER") , hostname );
	    printf(ANSI_YELLOW_BOLD);

	    cout << cwd;;
	    printf(ANSI_DEFAULT);
	    cout <<" $> ";

		getline(cin , command);

		command = trim_outer_spaces(command);
		
		// check for & at the end
		if(command[(int)command.size()-1] == '&'){
			is_background = true;
	        command.pop_back();
		}

		// split on the basis of pipe delimiter
		vector<string> commands = split(command, '|');


		// if no pipe is there
		if((int)commands.size() == 1){

			// the most general command is     cmd < input_file > output_file
			vector<string> parts = split_by_redirector(commands[0]);
			// if any is not present in input command then there is "" in corresponding index in 3parts vector
	        
	        if(parts.size()==0){
	            cout <<"Wrong redirection.\n";
	            return 1;
	        }
	        // cout <<"after splitting:";
	        // for(string s : parts) cout <<"["<<s<<"]"; 
	        // cout <<"\n";

			
			// cd command 
			if( split(commands[0], ' ')[0] == "cd" && split(commands[0],' ').size()==2 ){
				string dir = split(commands[0], ' ')[1];
				
				string usr_name = username;
				if(dir[0]=='~') dir = "/home/" + usr_name + dir.substr(1);  		// ~ means "/home/USERNAME"

				const char* c = dir.c_str();
				int val = chdir(c);
	            if(val < 0 ) cout <<"Wrong directory.\n";
			}


	        // exit command
	        else if(split(commands[0],' ').size()==1 && split(commands[0],' ')[0] == "exit"){
	            return 0;
	        }


	        // others
	        else{

				// create a child process
				pid_t pid = fork();

				//pid 0 corresponds to child process
				if(!pid){
					set_input_redirection(parts[1]);    //Redirection input 
					set_output_redirection(parts[2]);   //Redirection output
					execute_command(parts[0]);          //Execute the command using execvp
					exit(0);                            //Exit the child process
				}
	        }

	        // if not a background process    then wait for its execution to finish
	        // else dont wait and allow the user to give next command
	        if( ! is_background ){
	            wait(NULL);
	        }
		}	
		else{
	        if(command[0] == '|' ){
	            cout <<"Piping Error\n";
	            return 1;
	        }
	        int n = commands.size();            // number of commands seperated by pipe
	        int next_fd[2], prev_fd[2];         // creating array to store file descriptor of adjacent commands

	        for(int i=0; i<n; i++)
	        {
	            vector<string> parts = split_by_redirector(commands[i]);
	            if(i!=n-1)                   // Create new pipe except for the last command
	                pipe(next_fd);
	            
	            pid_t pid = fork();          // creating child process that is fork for every command
	            // in the child process
	            if(!pid){
	                if( !i || i==n-1){                    // for first and last commands
	                    set_input_redirection(parts[1]);  // redirecting input file
	                    set_output_redirection(parts[2]); // redirecting output file
	                }
	                if(i > 0){                           // reading from previous command for every command except the first command
	                    dup2(prev_fd[0],0), close(prev_fd[0]), close(prev_fd[1]);
	                }
	                if(i!=n-1){                            // writing into pipe for every command except the last command
	                    close(next_fd[0]), dup2(next_fd[1],1), close(next_fd[1]);
	                }
	                execute_command(parts[0]);            // executing  command
	            }
	            // in the parent process
	            if(i){
	                close(prev_fd[0]), close(prev_fd[1]);
	            }
	            if(i!=n-1)                              // copy the next_fd into prev_fd for every command except the last command
	                prev_fd[0] = next_fd[0], prev_fd[1] = next_fd[1];
	        }

	        // if no background process,then wait for its execution to finish
	        if(!is_background){
	            while(wait(NULL)>0);
	        }
		}
	}
	
}


int main(){
	
	display_help();

	int status = 1;
	while(status){
		status = shell_loop();
	}

	printf( ANSI_RED "\nGoodBye!!\ntype ./shell to run again\n\n");

	return 0;
}




// Function Definations ===============================================================



// removes the spaces from front and end of string

string trim_outer_spaces(const string &s){
	int n = s.length();
    if(n==0) return s;
	int i=0,j=n-1;
	while(i<n && s[i]==' ') i++;
	while(j>=0 && s[j]==' ') j--;
	return s.substr(i, j-i+1);
}

// splits a string into substrings based on a delimiter
vector<string> split(const string &s, char delimiter){
    vector<string> ans;

    // making a new string stream
    stringstream new_stream(s);
    
    string here;
    // read substrings from the new_stream seperated with delimiter and store it in here
    while(getline(new_stream, here, delimiter)){
    	if(here.size()==0) continue;
        ans.push_back(here);
    }
    
    return ans;
}

// will set the input redirections if s is not empty
void set_input_redirection(string& s){
	// Opening the input redirecting file
    if(s.size()>0){
        int inp_file_desc = open(s.c_str(),O_RDONLY);  // Open in read only mode and store the input file descriptor
        
        if(inp_file_desc < 0){
            cout<<"Error :"<<s<<endl;
            exit(1);
        }

        // Redirect the input using file descriptor
        if( dup2(inp_file_desc,0) < 0){
            cout<<"Error :"<<endl;
            exit(1);
        }
    }
}


// will set the output redirections if s is not empty

void set_output_redirection(string& s){
	// Opening the output redirecting file
    if(s.size()>0){
        int out_file_desc = open(s.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);  // Open in create and truncate mode and store the input file descriptor
        // Redirect output using file descriptor
        if( dup2(out_file_desc,1) < 0){
            cout<<"Error :"<<endl;
            exit(1);
        }
    }
}


// will take a command like    a < b > c  and split the strings a,b,c and return them in ans vector

vector<string> split_by_redirector(string s){
    vector<string> ans ( 3 );
    vector<string> empty;

    vector<string> parts1 = split(s , '<');

    if(s.back() == '<' || s.back() == '>') return empty;
    

    // a or a > c   case
    if(parts1.size() == 1){
        vector<string> parts2 = split(s , '>');
        
        // a case
        if(parts2.size() == 1){
            ans[0] = trim_outer_spaces(parts2[0]);
            if(ans[0]=="") return empty;
        }
        // a > c case
        else{
            ans[0] = trim_outer_spaces(parts2[0]);
            if(ans[0]=="") return empty;
            ans[2] = trim_outer_spaces(parts2[1]);
            if(ans[2]=="") return empty;
        }
    }
    // a < b  or  a < b > c   case
    else{
        vector<string> parts3 = split(parts1[1] , '>'); //parts1[1] is    b or b > c

        // a < b case
        if(parts3.size()==1){
            ans[1] = trim_outer_spaces(parts3[0]);
            if(ans[1]=="") return empty;
        }
        // a < b > c  case
        else{
            ans[1] = trim_outer_spaces(parts3[0]);
            if(ans[1]=="") return empty;
            ans[2] = trim_outer_spaces(parts3[1]);
            if(ans[2]=="") return empty;
            // cout <<"inp file = [" << ans[1] <<"]\n";
        }
        ans[0] = trim_outer_spaces(parts1[0]);
        if(ans[0]=="") return empty;
    }


    return ans;
}


// will take a command as a string and execute it using execvp syscall
void execute_command(string command){
    vector<string> arr , parts = split(command , ' ');

    for(auto & here: parts){
    	if(parts.size()>0){
    		arr.push_back(here);
    	}
    }

    int n = arr.size();
    char * arg[n+1]; // 1 more to accomodate NULL at the end
    arg[n]=NULL;
    
    for(int i=0;i<n;i++){
    	const char *c = arr[i].c_str();  	// .c_str converts a c++ string object to const char* pointer
    	arg[i] = const_cast<char*>(c);  	// const_cast is a casting operator
    }


    int val = execvp( arg[0] , arg );
    if(val==-1){
    	cout <<"Invalid Command.\n";
    }
}
