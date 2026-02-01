#include "crawler/spider.h"
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <csignal>
#include <atomic>

#include "services/url.h"
#include "services/producer_consumer.h"

using namespace crawler;
using namespace services::url;

// Global flag for signal handling
static std::atomic<components::Frontier*> g_frontier(nullptr);
static std::atomic<bool> g_stopRequested(false);

void signalHandler(int signum) {
    if (signum == SIGINT) {
		std::cout << "\n\nInterrupt signal received. Stopping frontier...\n"; 
        g_stopRequested.store(true);
    }
}

int main(int argc, char const *argv[])
{	

	components::FrontierBuilder builder;
	auto sharedURLQueue = std::make_shared<services::pattern::SharedQueue<crawler::types::URL>>();

	builder.setSharedURLQueue(sharedURLQueue);
	builder.setFrontQueuesSize(10);
	builder.setBackQueuesSize(10);
	builder.setPrioritizer(new components::SamplePrioritizer());
	builder.setFrontSelector(new components::SampleFrontSelector());
	builder.setRouter(new components::SampleRouter());
	builder.setBackSelector(new components::SampleBackSelector());

	std::unique_ptr<components::Frontier> frontier = builder.get();
	
	// Register signal handler for Ctrl+C (SIGINT)
	g_frontier.store(frontier.get());
	std::signal(SIGINT, signalHandler);

	frontier->run();

	frontier->insertToFrontQueue("www.google.com");
	frontier->insertToFrontQueue("https://www.yahoo.com");
	frontier->insertToFrontQueue("https://www.bing.com:8080");
	frontier->insertToFrontQueue("https://www.duckduckgo.com:notaport");
	frontier->insertToFrontQueue("baidu.com");
	frontier->insertToFrontQueue("http://ask.com/path?query=example#fragment");
	frontier->insertToFrontQueue("https://www.aol.com?search=query&page=1");
	frontier->insertToFrontQueue("https://www.ask.com/");
	frontier->insertToFrontQueue("https://nic.谷歌/");
	frontier->insertToFrontQueue("domain.co.uk/path/to/resource?param1=value1&param2=value2#section2");

	std::mutex m;
	
	std::vector<std::thread> threads;
	for (auto i{ 0 }; i < 10; ++i) {
		threads.emplace_back([&]() {
			std::optional<crawler::types::URL> url = sharedURLQueue->pop();
			{
				std::unique_lock<std::mutex> lock(m);
				if (url.has_value()) {
					std::cout << url->url() << '\n';

					std::cout << "  Scheme: " << url->scheme() << '\n';
					std::cout << "  Domain: " << url->domain() << '\n';
					std::cout << "  Port: " << url->port() << '\n';
					std::cout << "  Path: " << url->path() << '\n';
					std::cout << "  Fragment: " << url->fragment() << '\n';
					auto queryValue = url->query("query");
					if (queryValue.has_value()) {
						std::cout << "  Query 'query': " << queryValue.value() << '\n';
					}
				} 
			}
		});
	}

	for (auto& t: threads) {
		t.join();
	}

	while (!g_stopRequested.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Always stop the frontier, regardless of signal
	frontier->stop();

	return 0;
}
