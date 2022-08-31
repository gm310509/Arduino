/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.gm310509.reddit.servlet;

import com.gm310509.reddit.subreddit.Metric;
import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.google.gson.*;
import java.util.HashMap;

/**
 *
 * @author gm310509
 */
@WebServlet(name = "GetSubRedditMetric", urlPatterns = {"/GetSubRedditMetric"})
public class GetSubRedditMetric extends HttpServlet {

    /**
     * Processes requests for both HTTP <code>GET</code> and <code>POST</code>
     * methods.
     *
     * @param request servlet request
     * @param response servlet response
     * @throws ServletException if a servlet-specific error occurs
     * @throws IOException if an I/O error occurs
     */
    protected void processRequest(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        System.out.printf("GetSubRedditMetric: From: %s. QueryString: %s\n", request.getRemoteAddr(), request.getQueryString());

        String historyParam = request.getParameter("history");
        String subName = request.getParameter("subName");
//        System.out.println(String.format("name=%s", subName));
        if (subName != null) {
            subName = subName.toLowerCase();
        } else {
            subName = BackgroundStatsGatherer.DEFAULT_SUB_NAME;
        }

        boolean returnHistory = historyParam != null;
        Metric metric = new Metric("No Data");;
        HashMap<String, Metric> metricCache = (HashMap) getServletContext().getAttribute(BackgroundStatsGatherer.METRICS_CACHE_PARAM);
        if (metricCache != null) {
            Metric m = metricCache.get(subName);
            if (m != null) {
                metric = m;
            }
        } else {
            System.out.println("Creating a new \"no data\" metric.");
        }
        System.out.println(String.format("Metric: %s", metric.toString()));
        Gson gson = new Gson();
        String replyText;
        String contentType = "application/json;charset=UTF-8";
                        // Use "text/csv" if we return CSV data.
        if (returnHistory) {
            replyText = gson.toJson(metric);
        } else {
            Metric noHistMetric = new Metric(metric);
            replyText = gson.toJson(noHistMetric);
        }

        response.setContentType(contentType);
        try (PrintWriter out = response.getWriter()) {
            out.println(replyText);
        }
    }

    // <editor-fold defaultstate="collapsed" desc="HttpServlet methods. Click on the + sign on the left to edit the code.">
    /**
     * Handles the HTTP <code>GET</code> method.
     *
     * @param request servlet request
     * @param response servlet response
     * @throws ServletException if a servlet-specific error occurs
     * @throws IOException if an I/O error occurs
     */
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        processRequest(request, response);
    }

    /**
     * Handles the HTTP <code>POST</code> method.
     *
     * @param request servlet request
     * @param response servlet response
     * @throws ServletException if a servlet-specific error occurs
     * @throws IOException if an I/O error occurs
     */
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        processRequest(request, response);
    }

    /**
     * Returns a short description of the servlet.
     *
     * @return a String containing servlet description
     */
    @Override
    public String getServletInfo() {
        return "Short description";
    }// </editor-fold>

}
