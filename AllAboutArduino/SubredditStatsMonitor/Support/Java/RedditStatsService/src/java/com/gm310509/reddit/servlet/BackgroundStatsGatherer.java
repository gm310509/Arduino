/**
 * Background Stats Gatherer
 * -------------------------
 * 
 * Servlet that is loaded on startup to collect metrics from reddit.
 * 
 * V1.1.2.0 - 2022-08-25
 * - Added Exception handler to the Metric Helper to capture malformed
 *   JSON responses returned from reddit (for example, if the server is
 *   busy, it will return HTML, not JSON).
 * - Restructured the exception handling so that there are two levels,
 *   One for the document handling and one for the Sleep.
 * - Corrected hyperlink error in the index.html
 * - Added capture and display of the most recent exception (if any) along
 *   with a total number of exceptions caught in the metric helper.
 * 
 * V1.1.1.0 - 2022-08-25
 *   Added capability to load MetricHelpers based upon history files found
 *   in the history directory.
 *   Better handling of case and retention of case of the subreddit name
 *   found in the file name.
 * 
 * V1.1.0.0 - 2022-08-23
 *   Initial version - Only supports a single subreddit (arduino).
 * 
 */
package com.gm310509.reddit.servlet;

import com.gm310509.reddit.subreddit.Metric;
import com.gm310509.reddit.subreddit.MetricHelper;
import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.PrintWriter;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 *
 * @author gm310509
 */
@WebServlet(name = "BackgroundStatsGatherer", urlPatterns = {"/BackgroundStatsGatherer"}, loadOnStartup = 10)
public class BackgroundStatsGatherer extends HttpServlet {

    public static final String WEBAPP_VERSION = "1.1.2.0";
    public static final String HISTORY_PATH_PARAM = "RedditHistoryPath";
    public static final String METRICS_CACHE_PARAM = "metrics";
    public static final String DEFAULT_SUB_NAME = "arduino";
    
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
        System.out.println("BackgroundStatsGatherer: From: {}. QueryString: {}".format(request.getRemoteAddr(), request.getQueryString()));
        response.setContentType("text/html;charset=UTF-8");
        try (PrintWriter out = response.getWriter()) {
            /* TODO output your page here. You may use following sample code. */
            out.println("<!DOCTYPE html>");
            out.println("<html>");
            out.println("<head>");
            out.println("<title>Servlet BackgroundStatsGatherer</title>");
            out.println("  <style>");
            out.println("    body { font-family: \"Times New Roman\", Times, Serif; font-size:10pt; font-weight: normal;}");
            out.println("  </style>");
            out.println("</head>");
            out.println("<body>");
            
            out.println("  <h2>Reddit Monitor Web Service</h2>");
            out.println("  <p>");
            out.println(String.format("    Version: %s", WEBAPP_VERSION));
            out.println("  </p>");

            out.println("<h2>Metrics report</h2>");

            out.println("<table>");
            out.println("    <th>key</th><th>&nbsp;</th><th>Name</th><th>&nbsp;</th><th>Subscribers</th><th>&nbsp;</th><th>Active</th><th>&nbsp;</th><th>History</th>");
            out.println("  </tr>");
            HashMap<String, MetricHelper> metricCache = (HashMap) getServletContext().getAttribute(METRICS_CACHE_PARAM);
            for (String key : metricCache.keySet()) {
                Metric value = metricCache.get(key);
                out.println("    <tr>");
                out.println(String.format("      <td>%s</td><td>&nbsp;</td><td>%s</td><td>&nbsp;</td><td>%d</td><td>&nbsp;</td><td>%d</td><td>&nbsp;</td><td>%d</td>",
                        key, value.getName(), value.getSubscribers(), value.getActiveUsers(), value.getSubscriberHistory().size()));
                out.println("    </tr>");
            }
            out.println("</table>");

            out.println("<p>Context parameters</p>");
            out.println("<table>");
            out.println("    <th>key</th><th>&nbsp;</th><th>Value</th>");
            out.println("  </tr>");
            Enumeration<String> initParamKeys = getServletContext().getInitParameterNames();
            while (initParamKeys.hasMoreElements()) {
                String key = initParamKeys.nextElement();
                String value = getServletContext().getInitParameter(key);
                
                out.println(String.format("    <tr><td>%s</td><td>&nbsp;</td><td>%s</td></tr>", key, value));
            }
            out.println("</table>");

            out.println("<h2>Exceptions</h2>");
            out.println("<table>");
            out.println("    <th>key</th><th>&nbsp;</th><th>Count</th><th>&nbsp;</th><th>When</th><th>&nbsp;</th><th>Text</th>");
            out.println("  </tr>");
            for (String key : metricCache.keySet()) {
                MetricHelper value = metricCache.get(key);
                System.out.println("Key: %s = %s ".format(key, value)); 
                out.println("    <tr>");
                String dttm = "none";
                String msg = "n/a";
                if (value.getLastExceptionDateTime() != null) {
                    dttm = value.getLastExceptionDateTime().format(DateTimeFormatter.ISO_LOCAL_DATE_TIME);
                    msg = value.getLastException().toString();
                }
                out.println(String.format("      <td>%s</td><td>&nbsp;</td><td>%d</td><td>&nbsp;</td><td>%s</td><td>&nbsp;</td><td>%s</td>",
                        key, value.getExceptionCount(), dttm, msg));
                out.println("    </tr>");
            }
            out.println("</table>");

            out.println("</body>");
            out.println("</html>");
        }
    }
    
    private void outputMetric(PrintWriter out, Object o) {
        if (! (o instanceof MetricHelper)) {
            out.printf("<p>Wrong type of object (%s) for outputMetric\n", o.getClass().getCanonicalName());
            return;
        }
        
        MetricHelper metric = (MetricHelper) o;
        out.printf("<p>Name: %s<p>\n", metric.getName());
        out.printf("<p>Subscribers: %s<p>\n", metric.getSubscribers() != null ? metric.getSubscribers().toString() : "null");
        out.printf("<p>Active: %s<p>\n", metric.getActiveUsers()!= null ? metric.getActiveUsers().toString() : "null");
        
        out.println("<table>");
        out.println("  <tr>");
        out.println("    <th>date</th><th>&nbsp;</th><th>subscribers</th>");
        out.println("  </tr>");
        
        for (LocalDate date : metric.getSubscriberHistory().keySet()) {
            Integer subs = metric.getSubscriberHistory().get(date);
            String subsStr = subs != null? subs.toString() : "null";
            out.println("  <tr>");
            out.printf ("    <td>%s</td>&nbsp;<td></td><td>%s</td>\n", date.toString(), subsStr);
            out.println("  </tr>");
        }
        out.println("  </tr>");
        out.println("</table>");
        out.printf("<p>Total: %d records</p>\n", metric.getSubscriberHistory().size());
        out.printf("<p>File: %s</p>\n", metric.getHistoryFile().getAbsolutePath());
    }
    
    @Override
    public void init() throws ServletException {
            super.init(); //To change body of generated methods, choose Tools | Templates.
        System.out.println("********* Starting background stats gathering servlet.");

        HashMap<String, MetricHelper> metricCache = (HashMap) getServletContext().getAttribute(METRICS_CACHE_PARAM);
        if (metricCache == null) {
            metricCache = new HashMap<>();
            getServletContext().setAttribute(METRICS_CACHE_PARAM, metricCache);
        }
        String historyPathName = getServletContext().getInitParameter(HISTORY_PATH_PARAM);
        File historyPath = new File(historyPathName);
        Pattern subRedditFromHistoryFile = Pattern.compile("r-(.+?)-subscribers.txt$");
     
        System.out.println(String.format("\nDetecting subreddit history files in %s", historyPathName));
        for (File f : historyPath.listFiles(new FilenameFilter() {
            @Override       // Accept the file if it looks like "r-subname-subscribers.txt".
            public boolean accept(File dir, String name) {
                return name.startsWith("r-") && name.endsWith("-subscribers.txt");
            }
        }) ) {
            System.out.println(String.format("Found: %s", f.getAbsolutePath()));

            String fileName = f.getName();
            Matcher nameMatcher = subRedditFromHistoryFile.matcher(fileName);
            if (nameMatcher.find()) {
                String subName = nameMatcher.group(1);
                String subNameLower = subName.toLowerCase();
                System.out.println(String.format("%s - %s", fileName, subName));

                MetricHelper metric = metricCache.get(subNameLower);
                if (metric == null) {
                    System.out.println(String.format("new metricHelper(%s)", subName));
                    metric = new MetricHelper(subName);
                    
                    File historyFile = new File(historyPathName, fileName);
                    try {
                        int recCount = metric.loadHistory(historyFile);
                        System.out.printf("%d history records loaded.\n", recCount);
                    } catch (Exception e) {
                        System.out.printf("Exception loading history data from: %s\n", historyFile);
                        e.printStackTrace();
                    }
                    metricCache.put(subNameLower, metric);
                } else {
                    System.out.printf("Metric '%s' already exists \n", metric);
                }
            } else {
                System.out.println(String.format("%s - no match in regex", fileName));
            }
        }
    }

    @Override
    public void destroy() {
        super.destroy(); //To change body of generated methods, choose Tools | Templates.
        System.out.println("Terminating background stats gathering servlet. ************ ");
        HashMap<String, MetricHelper> metricCache = (HashMap) getServletContext().getAttribute(METRICS_CACHE_PARAM);
        for (String key : metricCache.keySet()) {
            MetricHelper metric = metricCache.get(key);
            metric.setShutdown(true);
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
