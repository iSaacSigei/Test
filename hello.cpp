#include <map>
#include <iostream>

template<typename K, typename V>
class interval_map {
    friend void IntervalMapTest();
    V m_valBegin;
    std::map<K, V> m_map;

public:
    // constructor associates whole range of K with val
    template<typename V_forward>
    interval_map(V_forward&& val)
        : m_valBegin(std::forward<V_forward>(val)) {}

    // Assign value val to interval [keyBegin, keyEnd).
    // Overwrite previous values in this interval.
    // Conforming to the C++ Standard Library conventions, the interval
    // includes keyBegin, but excludes keyEnd.
    // If !(keyBegin < keyEnd), this designates an empty interval, and assign must do nothing.
    template<typename V_forward>
    void assign(K const& keyBegin, K const& keyEnd, V_forward&& val)
        requires (std::is_same<std::remove_cvref_t<V_forward>, V>::value) {

        // Step 1: Handle empty interval case
        if (!(keyBegin < keyEnd)) {
            return;
        }

        // Step 2: Insert the value for keyBegin, if necessary
        auto itBegin = m_map.lower_bound(keyBegin);
        if (itBegin == m_map.end() || itBegin->first != keyBegin) {
            auto prev = (itBegin == m_map.begin()) ? m_valBegin : std::prev(itBegin)->second;
            if (prev != val) {
                m_map[keyBegin] = val;
            }
            itBegin = m_map.find(keyBegin);
        }

        // Step 3: Remove redundant entries in the range [keyBegin, keyEnd)
        auto itEnd = m_map.lower_bound(keyEnd);
        if (itEnd != m_map.begin() && std::prev(itEnd)->second == val) {
            itEnd = m_map.erase(std::prev(itEnd), itEnd);
        }

        // Step 4: Assign value to the specified range
        m_map.erase(std::next(itBegin), itEnd);  // Clear any overlapping entries

        // Ensure the value after the keyEnd retains its original value
        if (itEnd != m_map.end() && itEnd->first == keyEnd) {
            m_map[keyEnd] = itEnd->second;
        } else {
            m_map[keyEnd] = (itEnd == m_map.begin()) ? m_valBegin : std::prev(itEnd)->second;
        }

        // Clean up redundant entries to maintain canonical form
        auto itPrev = itBegin;
        if (itBegin != m_map.begin()) {
            --itPrev;
        }

        if (itPrev != m_map.end() && itPrev->second == itBegin->second) {
            m_map.erase(itBegin);
        }
    }
};

int main() {
    interval_map<int, char> imap('A');
    imap.assign(1, 3, 'B');  // Assign 'B' to [1, 3)
    imap.assign(4, 6, 'C');  // Assign 'C' to [4, 6)

    // Output for testing
    for (const auto& [key, value] : imap.m_map) {
        std::cout << key << " -> " << value << "\n";
    }

    return 0;
}
