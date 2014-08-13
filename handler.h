#ifndef HANDLER_H
#define HANDLER_H

template<typename Graph, typename mapping_type>
class handler {
    Graph& getGraph(){
        return graph;
    }

    Graph graph;
    mapping_type mapping;
};

#endif // HANDLER_H
