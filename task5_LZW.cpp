//decompress LZW 

//build basic dictionary by inserting the base codes


//first grab the top line. The number listed is the number of bits per code


//while !EOF grab n bits as the code, find the code in the dictionary

//if not in the dictionary add to the dictionary 

int main(){

	fstream myfile (filename);
	ofstream myofile;
	string ofile_name = filename;
	if(myfile.is_open())
    	{
		myofile.open (ofile_name);	
		codes = pow(2, bits); 
		string dictionary[codes], s = "";
		int encoding = 0, i;
		for(i =0; i < codes; i++){
			dictionary[i] = "";
		}

		dictionary[encoding++] = "*";
		dictionary[encoding++] = " ";
		dictionary[encoding++] = "-";
		dictionary[encoding++] = "0";
		dictionary[encoding++] = "1";
		dictionary[encoding++] = "2";
		dictionary[encoding++] = "3";
		dictionary[encoding++] = "4";
		dictionary[encoding++] = "5";
		dictionary[encoding++] = "6";
		dictionary[encoding++] = "7";
		dictionary[encoding++] = "8";
		dictionary[encoding++] = "9";
	
		while(getline(myfile,line)){ //loop through lines
			while(1){ //not end of line
				//grab the next code
				String out = dictionary[code];
				if(out.compare("") == 0)
					cout << "error, unknown code " << code << endl;
				cout << out;
				if(s.compare("") == 0){
					s = out;
				}
				else{
					s = s.append(out);
					dictionary[encoding++] = s;
					s = out;	
				}
		
			}	
		}		
	}
	else{
		cout << "failed to open file\n";
	}
}
