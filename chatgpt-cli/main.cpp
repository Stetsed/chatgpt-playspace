#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;

size_t write_to_string(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t total_size = size * nmemb;
    if (ptr == NULL) {
        std::cerr << "Error: NULL response from curl" << std::endl;
        exit(3);
    }
    std::string *response = (std::string *) stream;
    response->append((char *) ptr, total_size);
    return total_size;
}
std::string API(const string& name){
  char* ApiKey = getenv(name.c_str());
  if (ApiKey == NULL) {
    std::cerr << "Error: " << name << " environment variable is not set." << std::endl;
    exit(1);
  }
  return ApiKey;
}

int main(int argc, char** argv) {
  // Initialize CURL library
	std::vector<std::string> header_string = { 
		"Authorization: Bearer " + API("OPENAI_API_KEY"),
		"max_tokens: 512"
	};
  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Error initializing CURL library" << std::endl;
    return 1;
  }

  // Set up the request
  curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/engines/text-davinci-003/completions"); 
  curl_easy_setopt(curl, CURLOPT_POST, 1);

  // Prompt the user for a question
  std::string question;
 if (argc > 1) {
    // use the first command-line argument as the question
    question = argv[1];
  } else {
    std::cout << "enter a question: ";
    // use cin as the default input source
    std::getline(std::cin, question);
  }
  // Set the request body to the user's question
  std::string request_body = "{\"max_tokens\":512,\"prompt\": [\"" + question + "\"]}";
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());

  // Set the headers
  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
	 for (const auto &header : header_string) {
		headers = curl_slist_append(headers, header.c_str());
	}

  //headers = curl_slist_append(headers, header_string.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  // Set the callback function to handle the response
  std::string response;

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  // Send the request and receive the response
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "Error making request: " << curl_easy_strerror(res) << std::endl;
    return 1;
  }
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code >= 400) {
      std::cerr << "Error, HTTP response code: " << http_code << std::endl;
     return 1;
  }
  // Parse the response
  nlohmann::json response_json = nlohmann::json::parse(response);
  std::string answer = response_json["choices"][0]["text"];

  // Print the answer
  std::cout << answer << std::endl;
  // Clean up
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return 0;
}

