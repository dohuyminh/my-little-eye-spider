#include "crawler/spider.h"
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "services/url.h"

using namespace crawler;
using namespace service::url;

int main(int argc, char const *argv[])
{	

	component::FrontierBuilder builder;
	builder.setFrontQueuesSize(10);
	builder.setBackQueuesSize(10);
	builder.setPrioritizer(new component::SamplePrioritizer());
	builder.setFrontSelector(new component::SampleFrontSelector());
	builder.setRouter(new component::SampleRouter());
	builder.setBackSelector(new component::SampleBackSelector());

	component::Frontier frontier = builder.get();

	frontier.insertToFrontQueue("https://www.google.com");
	frontier.insertToFrontQueue("https://www.yahoo.com");
	frontier.insertToFrontQueue("https://www.bing.com");
	frontier.insertToFrontQueue("https://www.duckduckgo.com");
	frontier.insertToFrontQueue("https://www.baidu.com");
	frontier.insertToFrontQueue("https://www.ask.com");
	frontier.insertToFrontQueue("https://www.aol.com");
	frontier.insertToFrontQueue("https://www.ask.com");

	std::mutex m;
	
	std::vector<std::thread> threads;
	for (auto i{ 0 }; i < 8; ++i) {
		threads.emplace_back([&]() {
			std::optional<URL> url = frontier.popFront();
			{
				std::unique_lock<std::mutex> lock(m);
				std::cout << url->url() << '\n';
			}
		});
	}

	for (auto& t: threads) {
		t.join();
	}

	std::string url = "https://www.google.com/path/to/resource?query1=value1&query2=value2#section1";
	ParseResult result = parse(url);
	std::cout << "Scheme: " << result.scheme << "\n";
	std::cout << "Domain: " << result.domain << "\n";
	std::cout << "Subdomains: ";
	for (const auto& subdomain : result.subdomains) {
		std::cout << subdomain << " ";
	}
	std::cout << "\n";
	std::cout << "Port: " << result.port << "\n";
	std::cout << "Path: " << result.path << "\n";
	for (const auto& [key, value] : result.queryParams) {
		std::cout << "Query Parameter: " << key << "=" << value << "\n";
	}
	std::cout << "Fragment: " << result.fragment << "\n";

	return 0;
}
