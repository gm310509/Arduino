package com.gm310509.reddit.utility;

/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */


import com.google.gson.Gson;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringReader;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Base64;
import java.util.Map;

/**
 *
 * @author gm310509
 */
public class Token {
    
    private static Gson gson = new Gson();

    private String token = null;

    public Token() {
    }

    public Token(String tokenFileName) {
        this.tokenFileName = tokenFileName;
    }

    
    
    /**
     * Get the value of token
     *
     * @return the value of token
     */
    public String getToken() {
        return token;
    }

    /**
     * Set the value of token
     *
     * @param token new value of token
     */
    public void setToken(String token) {
        this.token = token;
    }

    @Override
    public String toString() {
        return token;
    }
    
    

    private String tokenFileName;

    /**
     * Get the value of tokenFileName
     *
     * @return the value of tokenFileName
     */
    public String getTokenFileName() {
        return tokenFileName;
    }

    /**
     * Set the value of tokenFileName
     *
     * @param tokenFileName new value of tokenFileName
     */
    public void setTokenFileName(String tokenFileName) {
        this.tokenFileName = tokenFileName;
    }

    

    private String getBasicAuthenticationHeader(String username, String password) {
        String valueToEncode = username + ":" + password;
        return "Basic " + Base64.getEncoder().encodeToString(valueToEncode.getBytes());
    }


    /* Potential repsonses from reddit:
    *  Wrong credentials or expired token:
    *   {"message": "Unauthorized", "error": 401}
    *
    *  User not authorised to access the registered "reddit App":
    *   {'error': 'invalid_grant'}
    *
    *  Token:
    *   {'access_token': 'eyJhbGciOiJSUzI1NiIsImtpZCI6IlNIQTI1NjpzS3dsMnlsV0VtMjVmcXhwTU40cWY4MXE2OWFFdWFyMnpLMUdhVGxjdWNZIiwidHlwIjoiSldUIn0.eyJzdWIiOiJ1c2VyIiwiZXhwIjoxNzA2ODYwMjA3LjY4OTM3OSwiaWF0IjoxNzA2NzczODA3LjY4OTM3OSwianRpIjoiR3JJeUNtdDQ0VFJmTjRHQjhUd3Ayd2QzR0k3emVBIiwiY2lkIjoiSkZCbkZuYU9hOU1yb1JlTGtKZTBGUSIsImxpZCI6InQyX3I4aDh2YnF4dCIsImFpZCI6InQyX3I4aDh2YnF4dCIsImxjYSI6MTcwNDIwMDQ2OTA0Mywic2NwIjoiZUp5S1Z0SlNpZ1VFQUFEX193TnpBU2MiLCJmbG8iOjl9.q5Wf5pc6ST0dwYGF14AUiVEQ_o7vJWePARlROM3OJCI3GcaBkUUibKz3awu32a1nuUp6E0Zpg69GQ-gZKpYUiRoqxnNQdsdLRXDVRyyc5qnrhMAWhpMphW6R62uPI_JQvv1CtrSAAz6WaE2Z6eBoTHhzSwdbH6lm3O9NItHMY588O4bK46rV2WfTC_D-dwRIKLzblC51Snn8IDgOWj154rY8XNApDrKtLOtazRPmWZWJWhAxZ2cvqfHWKgSfjS54ev3Q8Niwz8QQstByW8Sm9D_PPF9406hxyLT-fmxUA9CEXsZHB1lGPznReCDqwCGRuvtvntMJ09YRCBl4sBLOZg', 'token_type': 'bearer', 'expires_in': 86400, 'scope': '*'}
    */
    
    public String redditGetToken() {
        return redditGetToken(false);
    }
    
    public String redditGetToken(boolean force) {

        if (!force) {           // If we are not forcing a token retrieval, then 
            if (token != null) {    // check to see if we already have a token
//                System.out.println("Reusing existing token: " + token);
                return token;       // we do have a token, so return it.
            }
        
            if (tokenFileName != null) {
                Path tokenFilePath = Paths.get(tokenFileName);
                if (Files.exists(tokenFilePath)) {
//                    System.out.println(String.format("Attempting to read existing token from %s", tokenFileName));
                    try {
                        BufferedReader br = new BufferedReader(new FileReader(tokenFilePath.toFile()));
                        StringBuilder fileText = new StringBuilder();
                        String inLine;
                        while ((inLine = br.readLine()) != null) {
                            fileText.append(inLine);
                        }
                        token = fileText.toString();
                        return token;
                    } catch (FileNotFoundException e) {
                        System.out.println(String.format("token file %s not found - requesting new token", tokenFileName));
                    } catch (IOException e) {
                        System.out.println(String.format("IO Exception reading %s - requesting new token", tokenFileName));
                    }
                }
            }
//        } else {
//            System.out.println("Force is true, requesting a new token from the OAUTH2 server");
        }
        
        // Either we are forcing the request of a new token, or, we don't
        // already have one.
        System.out.println("requesting oauth2 token");
        
//        HttpClient client = HttpClient.newHttpClient();
        HttpClient client = HttpClient.newHttpClient();
        
        String oAuth2UrlText = "https://www.reddit.com/api/v1/access_token";
        
        String userId = "gm310509Service";
        String password = "m!bElu<_4m/>";
        String redditAppName = "gmcMod";
        //My app details provided by reddit after registration
        String redditClientId = "JFBnFnaOa9MroReLkJe0FQ";           // ~22 chars
        String redditClientPwd = "ai6ZJiSnWwxQXVRh-hp11ejVZi75Ig";   // ~30 chars
        String appName = String.format("%s/0.0.1", redditAppName);

        HttpRequest request = HttpRequest.newBuilder()
            .uri(URI.create(oAuth2UrlText))
            .header("Authorization", getBasicAuthenticationHeader(redditClientId, redditClientPwd))
            .header("User-Agent", appName)
            .POST(HttpRequest.BodyPublishers.ofString(String.format("grant_type=password&username=%s&password=%s", userId, password)))
            .build();
        
        try {
            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
            int status = response.statusCode();

            System.out.println(String.format("Status: %d", status));

            gson = new Gson();
            StringReader jsonStringReader = new StringReader(response.body());
            Map<?,?> map = gson.fromJson(jsonStringReader, Map.class);
            
            if (map.containsKey("message")) {
                String message = (String) map.get("message");
                Double errorCode = (Double) map.get("error");
                System.out.println("Error reply from reddit:");
                System.out.println("message: " + message);
                System.out.println("error: " + errorCode);
            } else if (map.containsKey("error")) {
                String errorMessage = (String) map.get("error");
                System.out.println("error: " + errorMessage);
            } else if (map.containsKey("access_token")) {
                System.out.println("Got a token!");
                token = (String) map.get("access_token");
                String tokenType = (String) map.get("token_type");
                Double expires = (Double) map.get("expires_in");
                String scope = (String) map.get("scope");
                System.out.println("token: " + token);
                System.out.println("type: " + tokenType + ", scope: " + scope + ", expires: " + expires);
            } else {
                System.out.println("*** Unrecognised oauth2 response");
                System.out.println("Response to oAuth2 request: ");
                System.out.println(response);
                System.out.println("------\nBody:");
                System.out.println(response.body());
                System.out.println(String.format("type: %s", response.getClass().getCanonicalName()));
            }

        } catch (IOException e) {
            System.out.println("IOException attempting to retrieve token.");
        } catch (InterruptedException e) {
            System.out.println("Interrupted Exception attempting to retrieve token.");
        }
        
        // Write the token out to our token file cache.
        try {
            if (tokenFileName != null) {
                Path tokenFilePath = Paths.get(tokenFileName);
                Files.deleteIfExists(tokenFilePath);

                BufferedWriter bw = new BufferedWriter(new FileWriter(tokenFilePath.toFile()));
                bw.write(token);
                bw.close();
            }
        } catch (IOException e) {
            System.out.println(String.format("IO Error writing token to token file: %s", tokenFileName));
            System.out.println(e.toString());
        }
        return token;
    }
}
