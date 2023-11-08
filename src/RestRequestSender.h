#include <HTTPClient.h>

String getInitData(String url) {
  HTTPClient http;
  String result = "error";

  try {
    // Send a GET request
    http.begin(url);
    int httpCode = http.GET();
    Serial.println("HTTP response code: " + String(httpCode));
    
    if (httpCode > 0) {
      result = http.getString();
      Serial.println("Response data: " + result);
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  } catch (const std::exception &e) {
    Serial.print("Exception caught: ");
    Serial.println(e.what());
    // Handle the exception here, e.g., log the error or take appropriate action
  }

  return result;
}

