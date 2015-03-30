#ifndef HANDLER_H
#define HANDLER_H
#include "mapping.h"
#include "archive.h"

template<typename Graph>
class archive_handle {

    typedef mapping<int,typename Graph::vertex_descriptor> mapping_type;

public:
    archive_handle( graph_archive<Graph>& archive,const Graph& g) : archive(archive),graph(g),rev(0){
    }
    archive_handle( graph_archive<Graph>& archive,const Graph& g, int _rev) : archive(archive),graph(g),rev(_rev){
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
    void commit(){
        rev = archive.commit(graph,map);
    }

    archive_handle checkout(int _rev) const {
        Graph graph = archive.checkout(_rev,map);
        FILE_LOG(logDEBUG4) << "handler: graph checked out";
        archive_handle handle(archive,graph,_rev);
        FILE_LOG(logDEBUG4) << "handle created";
        // Mapping?
        return handle;
    }

private:
    graph_archive<Graph>& archive;
    Graph graph;
    mapping_type map;
    int rev;
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
archive_handle<Graph> checkout(const archive_handle<Graph>& handle, int rev){
    return handle.checkout(rev);
}

#endif // HANDLER_H
