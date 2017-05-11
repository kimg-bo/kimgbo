#include "CircularBuffer.h"
#include <iostream>

using namespace kimgbo;
using namespace kimgbo::net;
using namespace std;


int main(int argc, char* argv[])
{
	char buf[256];
	for(int i=0; i<256; i++)
	{
		buf[i]='q';
	}
	CircularBuffer buffer;
	buffer.append(buf, 256);
	buffer.append(buf, 256);
	cout<<"w: " << buffer.writableBytes() << "r: " << buffer.readableBytes() << endl;
	
	cout<< buffer.retrieveAsString(50) << endl;
	cout<<"w: " << buffer.writableBytes() << "r: " << buffer.readableBytes() << endl;
	
	buffer.append(buf, 256);
	cout<<"w: " << buffer.writableBytes() << "r: " << buffer.readableBytes() << endl;
	
	
	return 0;
}