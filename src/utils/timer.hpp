/**
 * @file timer.hpp
 * @brief High-precision timing utility for measuring execution time of operations
 * @author Etienne Rosin
 * @date February 12, 2025
 * 
 * This utility provides classes for measuring and tracking execution time
 * of different operations in an application. It includes individual timer
 * functionality and management of multiple timers.
 */
#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
 
/**
 * @class Timer
 * @brief Measures elapsed time for specific tasks with millisecond precision
 * 
 * The Timer class provides functionality to measure elapsed time between
 * start and stop points. It can accumulate multiple timing intervals and
 * provides the total elapsed time.
 */
class Timer {
public:
    /**
     * @brief Default constructor
     * Creates a timer with the name "Unnamed Timer"
     */
    Timer() : m_name("Unnamed Timer"), m_startTime(), m_endTime(), m_running(false), m_elapsed(0) {}

    /**
     * @brief Constructor with custom name
     * @param name The name identifier for the timer
     */
    explicit Timer(const std::string& name) 
        : m_name(name), m_startTime(), m_endTime(), m_running(false), m_elapsed(0) {}

    /**
     * @brief Starts the timer
     * 
     * Records the current time as the start time. If the timer is already
     * running, this operation has no effect.
     */
    void start() {
        if (!m_running) {
            m_startTime = std::chrono::high_resolution_clock::now();
            m_running = true;
        }
    }

    /**
     * @brief Stops the timer
     * 
     * Records the current time as the end time and adds the interval to
     * the total elapsed time. If the timer is not running, this operation
     * has no effect.
     */
    void stop() {
        if (m_running) {
            m_endTime = std::chrono::high_resolution_clock::now();
            m_elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(m_endTime - m_startTime).count();
            m_running = false;
        }
    }

    /**
     * @brief Gets the total elapsed time
     * @return The total elapsed time in milliseconds
     * 
     * If the timer is currently running, includes the time since the last start.
     * If the timer is stopped, returns the accumulated total of all measured intervals.
     */
    long get_elapsed() const {
        if (m_running) {
            auto now = std::chrono::high_resolution_clock::now();
            return m_elapsed + std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
        }
        return m_elapsed;
    }

    /**
     * @brief Displays the timer's name and elapsed time
     * 
     * Outputs the timer information to standard output in the format:
     * "[name]: [elapsed] ms"
     */
    // void display() const {
    // //  std::cout << m_name << ": " << get_elapsed() << " ms" << std::endl;
    //     std::cout << m_name << ": " << get_elapsed() << " ms";
    // }
    void display() const {
        const size_t width = 31;
        const size_t inner_width = width - 2;
    
        auto center_text = [](const std::string& text, size_t column_width) {
            size_t padding = column_width - text.length();
            size_t left_pad = padding / 2;
            size_t right_pad = padding - left_pad;
            return std::string(left_pad, ' ') + text + std::string(right_pad, ' ');
        };
        // std::cout << "| " << center_text(m_name, inner_width) << get_elapsed() << " ms |\n";

        std::cout << "| " << std::left << std::setw(15) << m_name << ": " << std::setw(7) << get_elapsed() << " ms |\n";

        // std::cout << "+" << std::string(inner_width, '-') << "+\n"
        //           << "| " << std::left << std::setw(15) << m_name
        //           << std::right << std::setw(13) << get_elapsed() << " ms |\n"
        //           << "+" << std::string(inner_width, '-') << "+";
    }

private:
    std::string m_name;                                                        ///< Timer identifier name
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;   ///< Last start time point
    std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;     ///< Last end time point
    bool m_running;                                                            ///< Timer running state
    long m_elapsed;                                                            ///< Accumulated elapsed time in milliseconds
};

/**
 * @class Timers
 * @brief Manages multiple Timer objects and tracks total execution time
 * 
 * The Timers class provides a container for multiple Timer objects and
 * automatically maintains a "Total" timer that represents the sum of
 * all other timers.
 */
class Timers {
public:
    /**
     * @brief Constructor
     * 
     * Creates a Timers object and initializes it with a default "Total" timer
     */
    Timers() {
        add("Total");
    }

    /**
     * @brief Adds a new timer
     * @param name The name for the new timer
     * 
     * Creates and adds a new Timer object with the specified name.
     * If a timer with the given name already exists, it will be overwritten.
     */
    void add(const std::string& name) {
        m_timers[name] = Timer(name);
    }

    /**
     * @brief Accesses a timer by name
     * @param name The name of the timer to access
     * @return Reference to the requested Timer object
     * @throw std::out_of_range if the timer does not exist
     */
    Timer& operator()(const std::string& name) {
        return m_timers.at(name);
    }

    /**
     * @brief Displays timing information for all timers
     * 
     * Outputs the total time (sum of all timers except "Total")
     * followed by the individual times for each timer.
     * Format:
     * Total: [total_time] ms
     * [timer1_name]: [timer1_time] ms
     * [timer2_name]: [timer2_time] ms
     * ...
     */
    // void display() const {
    //     long total_time = 0;
        
    //     for (const auto& pair : m_timers) {
    //         const std::string& name = pair.first;
    //         const Timer& timer = pair.second;
    //         if (name != "Total") {
    //             total_time += timer.get_elapsed();
    //         }
    //     }

    //     std::cout << "Total: " << total_time << " ms" << std::endl;

    //     for (const auto& pair : m_timers) {
    //         const std::string& name = pair.first;
    //         if (name != "Total") {
    //             const Timer& timer = pair.second;
    //             timer.display();
    //             std::cout << std::endl;
    //         }
    //     }
    // }
    void display() const {
        const size_t width = 31;
        const size_t inner_width = width - 2;
    
        auto center_text = [](const std::string& text, size_t column_width) {
            size_t padding = column_width - text.length();
            size_t left_pad = padding / 2;
            size_t right_pad = padding - left_pad;
            return std::string(left_pad, ' ') + text + std::string(right_pad, ' ');
        };
    
        long total_time = 0;
        for (const auto& pair : m_timers) {
            if (pair.first != "Total") {
                total_time += pair.second.get_elapsed();
            }
        }
    
        // En-tête
        std::cout << "+" << std::string(inner_width, '-') << "+\n"
                  << "|" << center_text("Timer Summary", inner_width) << "|\n"
                  << "+" << std::string(inner_width, '-') << "+\n";
        
        // Total
        std::cout << "| " << std::left << std::setw(15) << "Total"
                  << std::right << std::setw(13) << total_time << " ms |\n";
        // << "+" << std::string(inner_width, '-') << "+\n";

        // Timers individuels
        for (const auto& pair : m_timers) {
            if (pair.first != "Total") {
                pair.second.display();
                std::cout << "\n";
            }
        }
        std::cout << "+" << std::string(inner_width, '-') << "+\n";
    }

private:
    std::unordered_map<std::string, Timer> m_timers;  ///< Container for all timer objects
};

