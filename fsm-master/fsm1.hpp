#pragma once
#define FSM_VERSION "1.0.0"

#include<algorithm>
#include<deque>
#include<functional>
#include<iostream>
#include<map>
#include<sstream>
#include<string>
#include<vector>

namespace fsm
{
    template<typename T>
    inline std::string to_string(const T &t)
    {
        std::stringstream ss;
        return ss<<t ? ss.str() : std::string();
    }

    template<>
    inline std::string to_string(const std::string &t)
    {
        return t;
    }

    typedef std::vector<std::string> args;
    typedef std::function<void (const fsm::args & args)> call;

    struct state
    {
        int name;
        fsm::args args;
        state(const int &name = 'null') : name(name){}

        state operator()()const
        {
            state self = *this;
            self.args = {};
            return self;
        }

        template<typename T0>
        state operator ()(const T0 &t0) const
        {
            state self = *this;
            self.args = {fsm::to_string(t0)};
        }

        template<typename T0, typename T1>
        state operator()(const T0 &t0, const T1 &t1) const
        {
            state self = *this;
            self.args = {fsm::to_string(t0), fsm::to_string(t1)};
            return self;
        }

        operator int () const
        {
            return name;
        }

        bool operator <(const state &other) const
        {
            return name < other.name;
        }

        bool operator == (const state &other) const
        {
            return name == other.name;
        }

        template<typename ostream>
        inline friend ostream &operator <<(ostream &out, const state &t)
        {
            if(t.name >= 256)
            {
                out << char((t.name >> 24) & 0xff);
                out << char((t.name >> 16) & 0xff);
                out << char((t.name >> 8) & 0xff);
                out << char((t.name >> 0) & 0xff);
            }
            else
            {
                out << t.name;
            }

            out << "(";
            std::string sep;
            for(auto &arg : t.args)
            {
                out << sep << arg;
                sep = ',';
            }
            out << ")";
            return out;
        }
    };

    typedef state trigger;

    struct transition
    {
        fsm::state previous, trigger, current;

        template<typename ostream>
        inline friend ostream &operator<<(ostream &out, const transition &t)
        {
            out << t.previous << " -> "<< t.trigger << "->" << t.current;
            return out;
        }
    };

    class stack
    {
    protected:
        void replace(fsm::state &current, const fsm::state &next)
        {
            call1(current, 'quit');
            current = next;
            call1(current,'init');
        }

        typedef std::pair<int, int> bistate;
        std::map<bistate, fsm::call> callbacks;

        mutable std::deque<fsm::transition> log;
        std::deque<fsm::state> deque;
        fsm::state current_trigger;

        typedef std::deque<fsm::state> states;
    
    public:
        stack(const fsm::state &start = 'null') : deque(1)
        {
            deque[0] = start;
            call1(deque.back(), 'init');
        }

        stack(int start) : stack(fsm::state(start)){}

        ~stack()
        {
            while(size())
            {
                pop();
            }
        }

        void push(const fsm::state &state)
        {
            if(deque.size() && deque.back() == state)
            {
                return;
            }

            call1(deque.back(), 'push');
            deque.push_back(state);
            call1(deque.back(), 'init');
        }

        void pop()
        {
            if(deque.size())
            {
                call1(deque.back(), 'quit');
                deque.pop_back();
            }
            if(deque.size())
            {
                call1(deque.back(), 'back');
            }
        }

        void set(const fsm::state &state)
        {
            if(deque.size())
            {
                replace(deque.back(), state);
            }
            else
            {
                push(state);
            }
        }

        size_t size() const
        {
            return deque.size();
        }

        fsm::state get_state(signed pos = -1) const
        {
            signed size = (signed)(deque.size());
            return size ? *(deque.begin() + (pos >= 0 ? pos % size : size - 1 + ((pos+1) %size))) : fsm::state();   
        } 

        fsm::transition get_log(signed pos = -1) const
        {
            signed size = (signed)(log.size());
            return size ? *( log.begin() + (pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ) ) : fsm::transition();
        }

        std::string get_trigger() const
        {
            std::stringstream ss;
            return ss << current_trigger, ss.str();
        }

        bool is_state(const fsm::state &state) const
        {
            return deque.empty() ? false : (deque.back() == state);
        }

        fsm::call &on(const fsm::state &from, const fsm::state &to)
        {
            return callbacks[bistate(from, to)];
        }

        bool call1(const fsm::state &from, const fsm::state &to) const
        {
            std::map<bistate, fsm::call>::const_iterator found = callbacks.find(bistate(from, to));
            if(found != callbacks.end())
            {
                log.push_back({from, current_trigger, to});
                if(log.size() > 50)
                {
                    log.pop_front();
                }
                found->second(to.args);
                return true;
            }
            return false;
        }

        bool command(const fsm::state &trigger)
        {
            size_t size = this->size();
            if(!size)
            {
                return false;
            }

            current_trigger = fsm::state();
            std::deque<states::reverse_iterator> aborted;
            for(auto it = deque.rbegin(); it != deque.rend(); ++it)
            {
                fsm::state &self = *it;
                if(!call1(self, trigger))
                {
                    aborted.push_back(it);
                    continue;
                }

                for(auto it = aborted.begin(), end = aborted.end(); it != end; ++it)
                {
                    call1(**it, 'quit');
                    deque.erase(--(it->base()));    //base
                }
                current_trigger = trigger;
                return true;
            }
            return false;
        }

        template<typename T>
        bool command( const fsm::state &trigger, const T &arg1 ) {
            return command( trigger(arg1) );
        }
        template<typename T, typename U>
        bool command( const fsm::state &trigger, const T &arg1, const U &arg2 ) {
            return command( trigger(arg1, arg2) );
        }

        template<typename ostream>
        ostream &debug( ostream &out ) {
            int total = log.size();
            out << "status {" << std::endl;
            std::string sep = "\t";
            for( states::const_reverse_iterator it = deque.rbegin(), end = deque.rend(); it != end; ++it ) {
                out << sep << *it;
                sep = " -> ";
            }
            out << std::endl;
            out << "} log (" << total << " entries) {" << std::endl;
            for( int i = 0 ; i < total; ++i ) {
                out << "\t" << log[i] << std::endl;
            }
            out << "}" << std::endl;
            return out;
        }

        // aliases
        bool operator()( const fsm::state &trigger ) {
            return command( trigger );
        }
        template<typename T>
        bool operator()( const fsm::state &trigger, const T &arg1 ) {
            return command( trigger(arg1) );
        }
        template<typename T, typename U>
        bool operator()( const fsm::state &trigger, const T &arg1, const U &arg2 ) {
            return command( trigger(arg1, arg2) );
        }
        template<typename ostream>
        inline friend ostream &operator<<( ostream &out, const stack &t ) {
            return t.debug( out ), out;
        }
        
    };
    
} 

#ifdef FSM_BUILD_SAMPLE2

// basic hfsm sample

#include <iostream>

// custom states (gerunds) and actions (infinitives)

enum {
    walking = 'WALK',
    defending = 'DEFN',

    tick = 'tick',
};

struct ant_t {
    fsm::stack fsm;
    int health, distance, flow;

    ant_t() : health(0), distance(0), flow(1) {
        // define fsm transitions: on(state,trigger) -> do lambda
        fsm.on(walking, 'init') = [&]( const fsm::args &args ) {
            std::cout << "initializing" << std::endl;
        };
        fsm.on(walking, 'quit') = [&]( const fsm::args &args ) {
            std::cout << "exiting" << std::endl;
        };
        fsm.on(walking, 'push') = [&]( const fsm::args &args ) {
            std::cout << "pushing current task." << std::endl;
        };
        fsm.on(walking, 'back') = [&]( const fsm::args &args ) {
            std::cout << "back from another task. remaining distance: " << distance << std::endl;
        };
        fsm.on(walking, tick) = [&]( const fsm::args &args ) {
            std::cout << "\r" << "\\|/-"[ distance % 4 ] << " walking " << (flow > 0 ? "-->" : "<--") << " ";
            distance += flow;
            if( 1000 == distance ) {
                std::cout << "at food!" << std::endl;
                flow = -flow;
            }
            if( -1000 == distance ) {
                std::cout << "at home!" << std::endl;
                flow = -flow;
            }
        };
        fsm.on(defending, 'init') = [&]( const fsm::args &args ) {
            health = 1000;
            std::cout << "somebody is attacking me! he has " << health << " health points" << std::endl;
        };
        fsm.on(defending, tick) = [&]( const fsm::args &args ) {
            std::cout << "\r" << "\\|/-$"[ health % 4 ] << " health: (" << health << ")   ";
            --health;
            if( health < 0 ) {
                std::cout << std::endl;
                fsm.pop();
            }
        };

        // set initial fsm state
        fsm.set( walking );
    }
};

int main() {
    ant_t ant;
    for(int i = 0; i < 12000; ++i) {
        if( 0 == rand() % 10000 ) {
            ant.fsm.push(defending);
        }
        ant.fsm.command(tick);
    }
}

#endif
