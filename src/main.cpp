#include "components/frontier/frontier.h"
#include "crawler/spider.h"
#include <iostream>

using namespace crawler;

int main(int argc, char const *argv[])
{	

	component::Frontier frontier(
		10,
		10,
		new component::SamplePrioritizer(), 
		new component::SampleFrontSelector(),
		new component::SampleRouter(), 
		new component::SampleBackSelector());

	frontier.insertToFrontQueue("https://www.google.com");
	frontier.insertToFrontQueue("https://www.yahoo.com");
	frontier.insertToFrontQueue("https://www.bing.com");
	frontier.insertToFrontQueue("https://www.duckduckgo.com");
	frontier.insertToFrontQueue("https://www.baidu.com");
	frontier.insertToFrontQueue("https://www.ask.com");
	frontier.insertToFrontQueue("https://www.aol.com");
	frontier.insertToFrontQueue("https://www.ask.com");

	while (auto url = frontier.popFront()) {
		std::cout << url->url() << std::endl;
	}

	return 0;
}
