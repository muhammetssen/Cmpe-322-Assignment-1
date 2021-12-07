struct arguments{
    string path<>;
	int a;
	int b;
};


program ADD_PROG{
	version ADD_VERS{
		int execute(arguments)=1;
	}=1;
}=0x01010101;