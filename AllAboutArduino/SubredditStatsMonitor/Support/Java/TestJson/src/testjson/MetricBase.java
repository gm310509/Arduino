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
public class MetricBase {

    public static int HISTORY_MAX = 365 * 4;        // four years of history.

    public MetricBase(String name) {
        this.name = name;
    }
    
    public MetricBase(Metric metric) {
        this.name = metric.getName();
        this.subscribers = metric.getSubscribers();
        this.active = metric.getActive();
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

    
    @Override
    public String toString() {
        return "MetricsFromReddit{" + "display_name=" + name + '}';
    }

}
