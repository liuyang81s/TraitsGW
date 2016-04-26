#include <iostream>
#include <json-c/json.h>

#include "http.h"
#include "gw.h"


using namespace std;

int main()
{
	//TRAITS_GW gw;
	TRAITS_GW gw("http://traits.imwork.net:10498/AnalyzeServer/system/");

	gw.init();
	cout << endl << endl;

	gw.hb();
	cout << endl << endl;

	gw.data();

	return 0;
}


