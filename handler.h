#ifndef HANDLER_H
#define HANDLER_H
#include "mapping.h"
#include "archive.h"

template<typename Graph>
class archive_handle {

    typedef mapping<internal_vertex,typename Graph::vertex_descriptor> vertex_mapping_type;
    typedef mapping<std::pair<internal_vertex,internal_vertex>,typename Graph::edge_descriptor> edge_mapping_type;

public:
    archive_handle( graph_archive<Graph>& archive,const Graph& g) : archive(archive),graph(g),rev(0){
    }
    archive_handle( graph_archive<Graph>& archive,const Graph& g, const revision& _rev) : archive(archive),graph(g),rev(_rev){
    }
    archive_handle( graph_archive<Graph>& archive) : archive(archive),graph(),rev(0){
    }
    archive_handle& operator=(const archive_handle& other){
        if (this != &other) // protect against invalid self-assignment
        {
        // 1: allocate new memory and copy the elements
            this->graph = other.graph;
            this->archive =other.archive;
            this->map = other.map;
            this->rev = other.rev;
        }
        // by convention, always return *this
        return *this;
    }
    Graph& getGraph() {
        return graph;
    }
    const Graph& getGraph() const{
        return graph;
    }
    const vertex_mapping_type& get_vertex_mapping() const {
        return map;
    }
    const edge_mapping_type& get_edge_mapping() const {
        return map;
    }
    void commit(){
        rev = archive.commit(graph,map,edge_map);
    }

    archive_handle checkout(const revision& _rev) const {
        Graph n;
        FILE_LOG(logDEBUG4) << "handle created";
        archive_handle handle(archive,n,_rev);
        Graph graph = archive.checkout(_rev,handle.map,handle.edge_map);
        handle.graph = graph;
        FILE_LOG(logDEBUG4) << "handler: graph checked out";
        // Mapping?
        return handle;
    }

private:
    graph_archive<Graph>& archive;
    Graph graph;
    vertex_mapping_type map;
    edge_mapping_type edge_map;
    revision rev;
};

template<typename archive_type, typename Graph>
archive_handle<Graph> commit(archive_type archive, Graph g){
    archive_handle<Graph> a = archive_handle<Graph>(archive,g);
    FILE_LOG(logDEBUG1) << "archive handle created";
    a.commit();
    FILE_LOG(logDEBUG1) << "archive handle changes commited";
    return a;
}

template<typename Graph>
archive_handle<Graph>& commit(archive_handle<Graph>& handle){
    handle.commit();
    return handle;
}

template<typename Graph>
archive_handle<Graph> checkout(const archive_handle<Graph>& handle, const revision& rev){
    return handle.checkout(rev);
}

#endif // HANDLER_H
