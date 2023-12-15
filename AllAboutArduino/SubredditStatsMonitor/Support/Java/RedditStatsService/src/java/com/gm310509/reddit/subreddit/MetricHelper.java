/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.gm310509.reddit.subreddit;

import static com.gm310509.reddit.subreddit.Metric.HISTORY_MAX;
import com.google.gson.Gson;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.format.DateTimeParseException;
import java.util.Map;
import java.util.TreeMap;

/**
 *
 * @author gm310509
 */
public class MetricHelper extends Metric implements Runnable {
    
    private static final int REFRESH_INTERVAL = 60000;
    private transient Thread helperThread;

    public MetricHelper(String name) {
        super(name);
        helperThread = new Thread(this);
        helperThread.start();
    }

    public MetricHelper(String name, Integer subscribers, Integer activeUsers) {
        super(name, subscribers, activeUsers);
        helperThread = new Thread(this);
        helperThread.start();
    }
    
    
    public void addHistory(LocalDate date, Integer subscribers) {
        TreeMap<LocalDate, Integer> subscriberHistory = getSubscriberHistory();
        subscriberHistory.put(date, subscribers);
        while (subscriberHistory.size() > HISTORY_MAX) {
            LocalDate removalKey = subscriberHistory.firstKey();
            subscriberHistory.remove(removalKey);
        }
    }

    // Cannot have a simpleDateFormat as it generates a weird multiple definitions
    // for JSON constant exception.
    // If needed, try using transient definition.
//    private SimpleDateFormat historyDateFormat = new SimpleDateFormat("yyyy-MM-dd");
    private transient File historyFile;
    /**
     * Load subscriber history from a file.
     * 
     * The file should be CSV with two fields. The fields are the date and the subscriber count.
     * The date should be "yyyy-MM-dd" format.
     * 
     * @param historyFile the file from which history is to be loaded.
     * @return the number of records loaded
     * @throws FileNotFoundException if the file cannot be found.
     * @throws IOException if there is a problem reading the file.
     */
    public int loadHistory(File historyFile) throws FileNotFoundException, IOException {
        this.historyFile = historyFile;                 // Save the file for when we need to save it back out.
        FileReader fr = new FileReader(historyFile);
        BufferedReader br = new BufferedReader(fr);
        String inLine;
        Integer lastSubs = null;            // Track the subscriber values
        int recCount = 0;
        while ((inLine = br.readLine()) != null) {
            String elements[] = inLine.split(",");
            try {
                if (elements.length == 2) {
                    LocalDate dt = LocalDate.parse(elements[0]);
                    Integer subs = Integer.parseInt(elements[1]);
                    addHistory(dt, subs);
                    lastSubs = subs;
                } else {
                    System.out.printf("Badly formatted record: %s\n", inLine);
                }
            } catch (DateTimeParseException e) {
                System.out.printf("Error parsing date text: %s\n", elements[0]);
            }
            recCount++;
        }
        if (lastSubs != null) {
            // If the most recent subscriber value is not null, then use it as the number of subscribers.
            setSubscribers(lastSubs);
        }
        return recCount;
    }

    public File getHistoryFile() {
        return historyFile;
    }
    
    
    private transient Throwable lastException = null;

    /**
     * Get the value of lastException
     *
     * @return the value of lastException
     */
    public Throwable getLastException() {
        return lastException;
    }
    
    private transient LocalDateTime lastExceptionDateTime = null;

    public LocalDateTime getLastExceptionDateTime() {
        return lastExceptionDateTime;
    }
    
    private transient int exceptionCount = 0;

    public int getExceptionCount() {
        return exceptionCount;
    }
    

    private transient boolean shutdown = false;

    /**
     * Get the value of shutdown
     *
     * @return the value of shutdown
     */
    public boolean isShutdown() {
        return shutdown;
    }

    /**
     * Set the value of shutdown
     *
     * @param shutdown new value of shutdown
     */
    public void setShutdown(boolean shutdown) {
        this.shutdown = shutdown;
        if (shutdown) {
            helperThread.interrupt();
        }
        System.out.printf("shutdown for helper thread %s set to %s\n", getName(), shutdown ? "true" : "false");
    }
    
    /**
     * Extract a number from the object if it is a map and has the contained key.
     * @param o an Object that should be a map.
     * @param key the key of the element to retrieve.
     * @return the value as an integer (or -1 if the value cannot be retrieved).
     */
    private int getNumFromReply(Object o, String key) {
        int result = -1;
        if (o instanceof Map) {
            Map dataMap = (Map) o;
            o = dataMap.get(key);
            if (o != null) {
                if (o instanceof Number) {
                    result = ((Number) o).intValue();
                }
            }
        } else {
            System.out.printf("Data element is not a map. It is a %s\n", o.getClass().getCanonicalName());
        }
        return result;
    }

    
    @Override
    public void run() {
        System.out.printf("Started refresh data thread for %s\n", getName());
        LocalDate prevDate = LocalDate.now();
        while (!shutdown) {
            try {
            // Connect to reddit to get stats.
                String urlText = String.format("https://www.reddit.com/r/%s/about.json", getName());
                URL url = new URL(urlText);

                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setRequestMethod("GET");
                conn.setConnectTimeout(5000);           // 5000 ms = 5 seconds
                conn.setReadTimeout(5000);
                conn.setRequestProperty("User-Agent", "statsMon/0.0.1");

                // Read reply.
                BufferedReader reader;
                String line;
                StringBuilder responseContent = new StringBuilder();
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

                if (status >= 300) {
                    System.out.println(responseContent.toString());
                }

                String jsonText = responseContent.toString();
                StringReader jsonStringReader = new StringReader(jsonText);
                Gson gson = new Gson();
                Map<?,?> map = gson.fromJson(jsonStringReader, Map.class);

                Object objData = map.get("data");

                int subscriberCount = getNumFromReply(objData, "subscribers");
                int activeUserCount = getNumFromReply(objData, "active_user_count");

                setSubscribers(subscriberCount);
                setActiveUsers(activeUserCount);
                LocalDate today = LocalDate.now();
                addHistory(today, subscriberCount);
                System.out.printf("Refreshed. Subs: %d, Active: %d\n", subscriberCount, activeUserCount);

                if(!today.equals(prevDate)) {
                    System.out.println("**** Date differs, writting it out to file: " + historyFile.getCanonicalPath());
                    prevDate = today;

                    File backupFile = new File(historyFile.getCanonicalPath() + ".bak");
                    if (backupFile.exists()) {
                        System.out.println("Backup file exists - removing");
                        backupFile.delete();
                    } else {
                        System.out.println("No backup file");
                    }
                    if (historyFile.exists()) {
                        System.out.printf("History file exists - renaming to backup: %s\n", backupFile.getCanonicalPath());
                        historyFile.renameTo(backupFile);
                    } else {
                        System.out.println("No history file");
                    }
                    FileWriter fw = new FileWriter(historyFile);
                    int cnt = 0;
                    for (LocalDate date : getSubscriberHistory().keySet()) {
                        Integer value = getSubscriberHistory().get(date);
                        String record = String.format("%s,%d\n", date.toString(), value);
                        fw.write(record);
                        cnt++;
                    }
                    fw.close();
                    System.out.printf("%d records written\n", cnt);
                }
            } catch (MalformedURLException e) {
                lastException = e;
                lastExceptionDateTime = LocalDateTime.now();
                exceptionCount++;
                System.out.println("Malformed URL Exception: " + e);
            } catch (ProtocolException e) {
                lastException = e;
                lastExceptionDateTime = LocalDateTime.now();
                exceptionCount++;
                System.out.println("Protocol Exception: " + e);
                e.printStackTrace();
            } catch (IllegalStateException e) {
                lastException = e;
                lastExceptionDateTime = LocalDateTime.now();
                exceptionCount++;
                System.out.println("Illegal State Exception: " + e);
                e.printStackTrace();
            } catch (IOException e) {
                lastException = e;
                lastExceptionDateTime = LocalDateTime.now();
                exceptionCount++;
                System.out.println("IO Exception: " + e);
                e.printStackTrace();
            } catch (Throwable t) {
                lastException = t;
                lastExceptionDateTime = LocalDateTime.now();
                exceptionCount++;
                System.out.println("Other exception: " + t);
                t.printStackTrace();
            }

            try {
//                System.out.printf("Sleeping %.2f seconds\n", REFRESH_INTERVAL / 1000.0);
                Thread.sleep(REFRESH_INTERVAL);
            } catch (InterruptedException e) {
            }
        }
        System.out.printf("** Stopping refresh data thread for %s\n", getName());
    }
 
}
