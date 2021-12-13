struct arguments{
    string path<>;
	int a;
	int b;
};
struct resultStruct{
    int result;
	bool error;
	string errorString<>;
};



program Executer{
	version ADD_VERS{
		resultStruct execute(arguments)=1;
	}=1;
}=0x01010101;