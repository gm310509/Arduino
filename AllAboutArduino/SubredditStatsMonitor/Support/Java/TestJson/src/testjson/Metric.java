/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package testjson;

import java.time.LocalDate;
import java.util.TreeMap;

/**
 *
 * @author gm310509
 */
public class Metric {

    public static int HISTORY_MAX = 365 * 4;        // four years of history.

    public Metric(String name) {
        this.name = name;
    }

    private String name;

    /**
     * Get the value of name
     *
     * @return the value of name
     */
    public String getName() {
        return name;
    }
    
    private Integer subscribers;

    /**
     * Get the value of subscribers
     *
     * @return the value of subscribers
     */
    public Integer getSubscribers() {
        return subscribers;
    }

    /**
     * Set the value of subscribers
     *
     * @param subscribers new value of subscribers
     */
    public void setSubscribers(Integer subscribers) {
        this.subscribers = subscribers;
    }

    private Integer active;

    /**
     * Get the value of active
     *
     * @return the value of active
     */
    public Integer getActive() {
        return active;
    }

    /**
     * Set the value of active
     *
     * @param active new value of active
     */
    public void setActive(Integer active) {
        this.active = active;
    }

    /**
     * Subscriber history information.
     */
    private TreeMap<LocalDate, Integer> history = new TreeMap();
    
    /**
     * Add a dated subscriber value to the history.
     * @param date the date.
     * @param subscribers the number of subscribers.
     */
    public void addHistory(LocalDate date, Integer subscribers) {
        history.put(date, subscribers);
    }

    public TreeMap<LocalDate, Integer> getHistory() {
        return history;
    }

    
    @Override
    public String toString() {
        return "MetricsFromReddit{" + "display_name=" + name + history.size() + '}';
    }

}
