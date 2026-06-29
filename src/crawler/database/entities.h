#pragma once

#include <future>
#include <utility>
namespace crawler::database {

template <typename ReturnType>
class ResultAsync {
public:
  
  std::future<ReturnType> getFuture() const noexcept {
    return fetchedData_.get_future();
  }  
 
  void setResult(ReturnType&& result) {
    fetchedData_.set_value(std::forward<ReturnType>(result));
  }

private:

  std::promise<ReturnType> fetchedData_;

};

}
