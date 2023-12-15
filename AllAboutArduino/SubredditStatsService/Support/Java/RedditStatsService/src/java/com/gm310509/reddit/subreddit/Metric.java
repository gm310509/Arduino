/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.gm310509.reddit.subreddit;

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
        this.subscribers = -1;
        this.activeUsers = -1;
    }

    public Metric(String name, Integer subscribers, Integer activeUsers) {
        this(name);
        this.subscribers = subscribers;
        this.activeUsers = activeUsers;
    }
    
    public Metric(Metric metric) {
        this (metric.getName(), metric.getSubscribers(), metric.getActiveUsers());
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

    private Integer activeUsers;

    /**
     * Get the value of activeUsers
     *
     * @return the value of activeUsers
     */
    public Integer getActiveUsers() {
        return activeUsers;
    }

    /**
     * Set the value of activeUsers
     *
     * @param activeUsers new value of activeUsers
     */
    public void setActiveUsers(Integer activeUsers) {
        this.activeUsers = activeUsers;
    }
    
    private TreeMap<LocalDate, Integer> subscriberHistory = new TreeMap();
    
    /**
     * Get the value of subscriberHistory
     *
     * @return the value of subscriberHistory
     */
    public TreeMap<LocalDate, Integer> getSubscriberHistory() {
        return subscriberHistory;
    }

//    /**
//     * Set the value of subscriberHistory
//     *
//     * @param subscriberHistory new value of subscriberHistory
//     */
//    public void setSubscriberHistory(TreeMap<Integer, Integer> subscriberHistory) {
//        this.subscriberHistory = subscriberHistory;
//    }
    
    @Override
    public String toString() {
        return "MetricsFromReddit{" + "display_name=" + name + ", subscribers=" + subscribers + ", active_user_count=" + activeUsers + ", history=" + subscriberHistory.size() + '}';
    }

}
