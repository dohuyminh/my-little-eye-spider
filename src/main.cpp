#include "crawler/spider.h"
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <csignal>
#include <atomic>
#include <random>

#include "services/url.h"
#include "services/producer_consumer.h"

using namespace crawler;
using namespace services::url;

// Global flag for signal handling
static std::atomic<components::Frontier*> g_frontier(nullptr);
static std::atomic<bool> g_stopRequested(false);

void signalHandler(int signum) {
    if (signum == SIGINT) {
		std::cout << "\n\nInterrupt signal received. Stopping all threads...\n"; 
        g_stopRequested.store(true);
    }
}

int main(int argc, char const *argv[])
{	

	components::FrontierBuilder builder;
	auto sharedURLQueue = std::make_shared<services::pattern::SharedQueue<crawler::types::URL>>();

	builder.setSharedURLQueue(sharedURLQueue);
	builder.setFrontQueuesSize(20);
	builder.setBackQueuesSize(20);
	builder.setPrioritizer(new components::RoundRobinPrioritizer(20));
	builder.setFrontSelector(new components::RoundRobinFrontSelector());
	builder.setRouter(new components::RoundRobinBackRouter(20));
	builder.setBackSelector(new components::RoundRobinBackSelector());

	std::unique_ptr<components::Frontier> frontier = builder.get();
	
	// Register signal handler for Ctrl+C (SIGINT)
	g_frontier.store(frontier.get());
	std::signal(SIGINT, signalHandler);

	frontier->run();

	// Test URLs for random selection
	std::vector<std::string> testURLs = {
		"www.google.com",
		"https://www.yahoo.com",
		"https://www.bing.com:8080",
		"baidu.com",
		"http://ask.com/path?query=example#fragment",
		"https://www.aol.com?search=query&page=1",
		"https://www.ask.com/",
		"https://www.github.com/user/repo",
		"domain.co.uk/path/to/resource?param1=value1&param2=value2#section2",
		"https://www.example.org/page"
	};

	std::mutex printMutex;
	
	// Create 20 producer threads
	std::vector<std::thread> producerThreads;
	for (auto i = 0; i < 20; ++i) {
		producerThreads.emplace_back([&frontier, &testURLs, &printMutex]() {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> urlDist(0, testURLs.size() - 1);
			std::uniform_int_distribution<> sleepDist(5000, 10000);  // 5-10 seconds in ms
			
			while (!g_stopRequested.load()) {
				// Insert random URL
				const auto& randomURL = testURLs[urlDist(gen)];
				frontier->insertToFrontQueue(randomURL);
				
				{
					std::unique_lock<std::mutex> lock(printMutex);
					std::cout << "[Producer] Inserted: " << randomURL << '\n';
				}
				
				// Sleep for random time (5-10 seconds)
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));
			}
		});
	}

	// Create 1 consumer thread that extracts from shared queue
	std::thread consumerThread([&sharedURLQueue, &printMutex]() {
		while (!g_stopRequested.load()) {
			std::optional<crawler::types::URL> url = sharedURLQueue->pop();
			
			if (url.has_value()) {
				{
					std::unique_lock<std::mutex> lock(printMutex);
					std::cout << "[Consumer] Extracted URL: " << url->url() << '\n';
					std::cout << "  Scheme: " << url->scheme() << '\n';
					std::cout << "  Domain: " << url->domain() << '\n';
					std::cout << "  Port: " << url->port() << '\n';
					std::cout << "  Path: " << url->path() << '\n';
					std::cout << "  Fragment: " << url->fragment() << '\n';
				}
				
				// Sleep for 5 seconds
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
	});

	// Wait for stop signal or until threads finish
	while (!g_stopRequested.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	{
		std::unique_lock<std::mutex> lock(printMutex);
		std::cout << "\nWaiting for all threads to finish...\n";
	}

	// Join all producer threads
	for (auto& t : producerThreads) {
		if (t.joinable()) {
			t.join();
		}
	}

	// Join consumer thread
	if (consumerThread.joinable()) {
		consumerThread.join();
	}

	// Stop frontier
	frontier->stop();

	{
		std::unique_lock<std::mutex> lock(printMutex);
		std::cout << "All threads stopped. Program exiting gracefully.\n";
	}

	return 0;
}
