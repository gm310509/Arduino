/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package testjson;


import com.google.gson.Gson;
import java.io.BufferedReader;
import java.io.File;
import java.io.FilenameFilter;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.time.LocalDate;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *
 * @author gm310509
 */
public class TestJson {
     private static Gson gson = new Gson();
    
    public void dump(String txt, Metric metric) {
        System.out.println(txt);
        System.out.println(gson.toJson(metric));
        System.out.println();
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        TestJson tj = new TestJson();
//        tj.goJson();
//        tj.listMonitorFiles();
        tj.testHash();
    }
    
    private Pattern subRedditFromHistoryFile = Pattern.compile("r-(.+?)-subscribers.txt$");
    
    public void listMonitorFiles() {
        File historyPath = new File("c:\\\\cygwin64\\\\home\\\\gm310509");
     
        System.out.println("File list (no filter)");
        for (File f : historyPath.listFiles()) {
            String fileName = f.getName();
            Matcher nameMatcher = subRedditFromHistoryFile.matcher(fileName);
            if (nameMatcher.find()) {
                String subredditName = nameMatcher.group(1).toLowerCase();
                System.out.println(String.format("%s - %s", fileName, subredditName));
            } else {
                System.out.println(String.format("%s - no match", fileName));
            }
        }

        System.out.println("\nFile list (with filter)");
        for (File f : historyPath.listFiles(new FilenameFilter() {
            @Override
            public boolean accept(File dir, String name) {
                return name.startsWith("r-") && name.endsWith("-subscribers.txt");
            }
        }) ) {
            System.out.println(String.format("%s", f.getAbsolutePath()));
        }
    }
    
    public void goJson() {
        Metric metric = new Metric("test");
        dump("after creation", metric);
        
        metric.addHistory(LocalDate.now(), 1);
        dump("with one history:", metric);
        
        metric.addHistory(LocalDate.now().minusDays(1), 2);
        metric.addHistory(LocalDate.now().minusDays(2), 3);
        dump("with three history:", metric);
        
        metric.setActive(42);
        dump("with three history:", metric);

        System.out.println("\n**************************************************************\n");
        Gson gson = new Gson();
        try {
//            FileReader fr = new FileReader("/cygwin64/home/gm310509/responseStats.json");
//            BufferedReader br = new BufferedReader(fr);
//            StringBuffer jsonTxt = new StringBuffer();
//            String inLine;
//            while ((inLine = br.readLine()) != null) {
//                jsonTxt.append(inLine);
//            }
            String urlText = "https://www.reddit.com/r/Arduino/about.json";
            URL url = new URL(urlText);
            
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            conn.setConnectTimeout(5000);           // 5000 ms = 5 seconds
            conn.setReadTimeout(5000);
            conn.setRequestProperty("User-Agent", "statsMon/0.0.1");
            
            BufferedReader reader;
            String line;
            StringBuffer responseContent = new StringBuffer();
            int status = conn.getResponseCode();
            if (status >= 300) {
                reader = new BufferedReader(new InputStreamReader(conn.getErrorStream()));
            }
            else {
                reader = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            }

            while ((line = reader.readLine()) != null) {
                    responseContent.append(line);
            }
            reader.close();

            String jsonText = responseContent.toString();
//            System.out.println(jsonText);
            StringReader jsonStringReader = new StringReader(jsonText);
            Map<?,?> map = gson.fromJson(jsonStringReader, Map.class);
            System.out.println(map);

            Object objData = map.get("data");
            System.out.println("-------------------------------------------");
            System.out.println(objData);
            System.out.println("-------------------------------------------");
            int subscriberCount = -1;
            if (objData instanceof Map) {
                Map dataMap = (Map) objData;
                System.out.println("objData is a map");
                Object o = dataMap.get("subscribers");
                if (o != null) {
                    if (o instanceof Number) {
                        subscriberCount = ((Number) o).intValue();
                    }
                }
                System.out.printf("map[subscribers] %d (%s)\n", subscriberCount, dataMap.get("subscribers"));
                System.out.printf("map[active_user_count] %s\n", dataMap.get("active_user_count"));
            } else {
                System.out.println("Data element is not a map??");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        LocalDate today = LocalDate.now();
        System.out.printf("Today: %s\n", today.toString());
        LocalDate prevDate = LocalDate.now();
        
        if (prevDate.equals(today)) {
            System.out.println("Prev date is equal to today.");
        } else {
            System.out.println("Prev date differs from today");
        }
        LocalDate yesterday = today.minusDays(1);
        if (yesterday.equals(today)) {
            System.out.println("yesterday is equal to today.");
        } else {
            System.out.println("yesterday differs from today");
        }
        
    }
  
    private void dumpHash(HashMap<String, String> hashMap) {
        Iterator<String> iter = hashMap.keySet().iterator();
        
        while (iter.hasNext()) {
            String key = iter.next();
            String value = hashMap.get(key);
            System.out.println(String.format("%s='%s'", key, value));
        }
        System.out.println(String.format("Total: %d elements", hashMap.size()));
    }
    
    
    private void testHash() {
        HashMap<String, String> hashMap = new HashMap<>();
        
        hashMap.put("Key 1", "value 1");
        hashMap.put("Key 2", "value 2");
        dumpHash(hashMap);
        System.out.println("Changing Key 1");
        hashMap.put("Key 1", "New value");
        dumpHash(hashMap);
    }
}
  