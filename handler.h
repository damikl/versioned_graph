#ifndef HANDLER_H
#define HANDLER_H
#include <boost/bimap.hpp>
#include "archive.h"

template<typename internal_type,typename external_type>
class mapping{
    typedef boost::bimap<internal_type,external_type> mapping_type;
public:
    const internal_type& get_internal_id(const external_type& id) const {
        return map.left.at(id);
    }
    internal_type& get_internal_id(const external_type& id) {
        return map.left.at(id);
    }

    const external_type& get_external_id(const internal_type& id) const {
        return map.right.at(id);
    }
    external_type& get_external_id(const internal_type& id) {
        return map.right.at(id);
    }
    void insert(const internal_type& iternal_id, const external_type& external_id){
        map.insert( typename mapping_type::value_type(iternal_id, external_id) );
    }
    bool erase_internal(const internal_type& id){
        std::size_t n = map.right.erase(id);
        if(n==1){
             return true;
        } else {
            assert(n==0);
            return false;
        }
    }
    bool erase_external(const external_type& id){
        std::size_t n = map.left.erase(id);
        if(n==1){
             return true;
        } else {
            assert(n==0);
            return false;
        }
    }
private:
    mapping_type map;
};

template<typename Graph>
class archive_handle {

    typedef mapping<int,typename Graph::vertex_descriptor> mapping_type;

public:
    archive_handle(graph_archive<Graph>& archive,Graph& g) : archive(archive),graph(g),rev(0){
    }
    archive_handle(graph_archive<Graph>& archive) : archive(archive),graph(),rev(0){
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
        FILE_LOG(logDEBUG4) << "graph checked out";
        archive_handle handle(archive,graph);
        FILE_LOG(logDEBUG4) << "handle created";
        handle.rev = _rev;
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
