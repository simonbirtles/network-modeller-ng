#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include <map>
#include <assert.h>
#include <unordered_set>
#include "ServiceRegistryCommon.h"

namespace NM
{
	namespace Registry
	{

		class Edge;

			/**
			* Node
			*
			*
			*/
			class Node
			{
			public:
				Node(GRAPHID GraphID):
					_graphID(GraphID)
				{};
        
				~Node(){};
            
				GRAPHID					_graphID;
				::std::vector<Edge*>	_inedges;        /// in / out related to the edges view (we have a node who is dependant on this node connected to this edge)
				::std::vector<Edge*>	_outedges;       /// in / out related to the edges view (this node is dependant on ....)

			private:
				Node(const Node&);
				Node& operator=(const Node&);
			};
			/**
			* Edge
			*
			*
			*/
			class Edge
			{
			public:
				Edge(GRAPHID GraphID):
					_graphID(GraphID)
				{
				};

				~Edge(){};

				GRAPHID	 _graphID;
				Node*	 _innode;                        /// in / out related to the edges view
				Node*	 _outnode;                       /// in / out related to the edges view

			private:
				Edge(const Edge&);
				Edge& operator=(const Edge&);
			};  

			/**
			 * ServiceDependencyGraph (directed graph)
			 *
			 * useful for getting circular dependencies check and 
			 * service shutdown working back up the tree to shutdown/deRegister 
			 * in order of no dependants
			 *
			 */
			class ServiceDependencyGraph
			{
			public:
				/**
				*
				*
				*
				*/
				ServiceDependencyGraph():
					_nextGraphID(1)						// starts at 1, 0 is invalidID
				{
				};
				/**
				*
				*
				*
				*/
				~ServiceDependencyGraph()
				{
					//OutputDebugString(L"ServiceDependencyGraph::dtor() \n");			

					// delete all node and edge classes - no particular order
					// just delete them all
					::std::vector<Node*>::iterator nodeIT = _nodes.begin();
					while( nodeIT != _nodes.end() )
					{
						delete *nodeIT;
						*nodeIT = nullptr;
						++nodeIT;
					}
					_nodes.clear();
					_id_to_nodeptr.clear();

					// now do edges 
					::std::vector<Edge*>::iterator edgeIT = _edges.begin();
					while( edgeIT != _edges.end() )
					{
						delete *edgeIT;
						*edgeIT = nullptr;
						++edgeIT;
					}
					_edges.clear();
					_id_to_edgeptr.clear();

				};
				/**
				* CreateServiceNode
				*
				* 
				*/
				GRAPHID CreateServiceNode()
				{
					//OutputDebugString(L"ServiceDependencyGraph::CreateServiceNode()\n");
					GRAPHID newNodeID = _nextGraphID++;
					Node* n = nullptr;
					n = new Node(newNodeID);
					if(n)
					{
						// save this new node.
						_nodes.push_back(n);
						// save the id to ptr 
						_id_to_nodeptr[newNodeID] = n;
						return newNodeID;
					}
					return 0;	// bad id.
				};
				/**
				*
				*
				*
				*/
				bool DeleteServiceNode(GRAPHID graphID)
				{
					//OutputDebugString(L"ServiceDependencyGraph::DeleteServiceNode() ");
					Node* dNode = GetNodePtr(graphID);

					// look at nodes that are dependant on dNode 
					// if we have nodes that are dependant on this... can we really delete it?
					// they would have to be shutdown at min but we dont have this info
					// so we expect that this has been taken care of and other deletes have been called
					// on the dependants and this means we should have no dependants left and therefore 
					// we can continue, if we do have some then return false (not deleted)
					if( dNode->_inedges.size() > 0)
					{
						// has dependants, cant delete it
						OutputDebugString(L"\n\t node has dependants - unable to delete..\n");
						return false;
					}
			
					// look at the nodes that dNode is dependant on and delete
					::std::vector<Edge*>::iterator dNodeOutEdges = dNode->_outedges.begin();
					while(dNodeOutEdges != dNode->_outedges.end() )
					{
						OutputDebugString(L"\n\t deleting links to own dependants");
						// dNode connected outedge to a node we are dependant on
						Edge* e = *dNodeOutEdges;
						// this node is the node that dNode is dependant on (the other side of the edge)
						Node* edgeOutNode = e->_outnode;
		#ifdef _DEBUG
						// this node should be dNode
						Node* edgeinNode = e->_innode;		
						assert( edgeinNode == dNode);
		#endif					
						// in the other connected Node (the one dNode is dependant on, find the connecting edge ptr and delete from the Node references
						::std::vector<Edge*>::iterator edgeOutNodeIT = ::std::find(edgeOutNode->_inedges.begin(), edgeOutNode->_inedges.end(), e);
						if( edgeOutNodeIT != edgeOutNode->_inedges.end())
						{
							edgeOutNode->_inedges.erase(edgeOutNodeIT);
						}

						// delete this edge now references to this edge are deleted in dependant node.
						// find it in the _edges then delete
						::std::vector<Edge*>::iterator edgeIT = _edges.begin();
						edgeIT = ::std::find(_edges.begin(), _edges.end(), e);
						if( edgeIT != _edges.end() )
						{
							OutputDebugString(L"\n\t deleting edge link");
							delete e;
							e = nullptr;	
							_edges.erase(edgeIT);
						}
		#ifdef _DEBUG
						else
						{
							assert(false);
						}
		#endif
						++dNodeOutEdges;
					}
			
					// delete ID to Node* mapping
					::std::map<GRAPHID, Node*>::iterator nodeIT = _id_to_nodeptr.find(graphID);
					if( nodeIT != _id_to_nodeptr.end() )
					{
						OutputDebugString(L"\n\t delete gid to Node* mapping");
						_id_to_nodeptr.erase( nodeIT );
					}

					// delete dNode			
					::std::vector<Node*>::iterator it = ::std::find(_nodes.begin(), _nodes.end(), dNode);
					if( it != _nodes.end() )
					{
						OutputDebugString(L"\n\t delete Node..");
						delete *it;
						*it = nullptr;
						_nodes.erase(it);
					}

					return true;
				};
				/**
				* GetLeafServices (public)
				*
				* Leaf services are services which have no dependants, therefore are the last objects on the tree
				* This function returns the current leaves on the tree.
				*/
				::std::unordered_set<GRAPHID> GetLeafServices()
				{
					// 1. find all roots
					::std::unordered_set<GRAPHID> roots = GetGraphTreeRoots();

					// 2. use each root and find all leaves of that rooted tree
					::std::unordered_set<GRAPHID>::iterator rootIT = roots.begin();
					::std::unordered_set<GRAPHID> leafs;
					// for each root 
					while( rootIT != roots.end() )
					{
						// 3. func will add each leaf to a ::std::unordered_set (no duplicates allowed)	
						GetGraphTreeLeafs( GetNodePtr(*rootIT), leafs);
						++rootIT;
					}
					// 4. return leafs
					return ::std::move(leafs);
				};
				/**
				* GetAncestorsServices (public)
				*
				* Given a Node ID, return all ancestors, i.e. Services that this Node is dependant on...
				* 
				*/
				::std::unordered_set<GRAPHID> GetAncestorsServices(GRAPHID GraphID)
				{
					::std::unordered_set<GRAPHID> ancestors;

					GetGraphNodeAncestors(GetNodePtr(GraphID), ancestors);

					// delete current node as it gets added but should not be in there.
					::std::unordered_set<GRAPHID>::iterator it = ancestors.find( GraphID );
					if( it != ancestors.end() )
						ancestors.erase(it);

					return ancestors;
				}
				/**
				* GetDescendantServices (public)
				*
				* Given a Node ID, return all children nodes, i.e. Services that are dependant on this Node...
				* 
				*/
				::std::unordered_set<GRAPHID> GetDescendantServices(GRAPHID GraphID)
				{
					::std::unordered_set<GRAPHID> children;

					GetGraphNodeChildren(GetNodePtr(GraphID),  children);

					// delete current node as it gets added but should not be in there.
					::std::unordered_set<GRAPHID>::iterator it = children.find( GraphID );
					if( it != children.end() )
						children.erase(it);

					return children;		
				}
				/**
				* GetChildServices (public)
				*
				* Given a Node ID, return direct child nodes, i.e. Services that are dependant on this Node...
				* 
				*/
				::std::unordered_set<GRAPHID> GetChildServices(GRAPHID GraphID)
				{
					::std::unordered_set<GRAPHID> children;

					Node* pNode = GetNodePtr(GraphID);
					size_t t = pNode->_inedges.size();
					for(size_t i=0; i<t; ++i)
					{
						children.insert( pNode->_inedges[i]->_innode->_graphID );
					}

					return children;		
				}
				/**
				* SetServiceDependencies
				*
				*
				* TODO fail if we get a circular dependancy 
				*/
				void SetServiceDependencies(GRAPHID GraphID, const ::std::vector<GRAPHID>& serviceDependencies )
				{
					Node* n = GetNodePtr(GraphID);			
					// create an edge and connect it to this node and node dependant on
					size_t dCount = serviceDependencies.size();
					for(size_t i=0; i<dCount; ++i)
					{
						Node* depNode = GetNodePtr(serviceDependencies[i]);
						if( depNode )
						{
							Edge* e = new Edge(_nextGraphID++);
							if( e )
							{
								OutputDebugString(L"\n\t-- Add connection to node dependancy ");
								// in / out related to the edges view
								e->_innode = n;					/// the edge uni direction is innode->outnode, so n (innode) is dependant on depNode (outnode)
								e->_outnode = depNode;			/// the edge uni direction is innode->outnode, so n (innode) is dependant on depNode (outnode)
								// add node
								_edges.push_back(e);
								// add to ID to Edge map
								_id_to_edgeptr[e->_graphID] = e;
								// add as an out edge (connected dependency) to the node 'n'
								n->_outedges.push_back(e);
								// add as an inedge to the node that the node 'n' is dependant on
								depNode->_inedges.push_back(e);
							}
						}
					}
					return;
				};

			private:
				/**
				* circular dependancy check...
				*
				* Bottom -> Up DFS search of a tree from a given node saving all nodes visited 
				* therefore saving all ansectors through to the root.
				* 
				*/
				//void GetGraphNodeAncestors(Node* pNode, ::std::unordered_set<GRAPHID> &ancestors)
				//{  
				//	// if this node has parents (is dependant on ...)
				//	if(pNode->_outedges.size() != 0)
				//	{
				//		::std::vector<Edge*>::iterator edgeIT = pNode->_outedges.begin();
				//		while( edgeIT != pNode->_outedges.end() )
				//		{
				//			GetGraphNodeAncestors( (*edgeIT)->_outnode, ancestors);
				//			++edgeIT;
				//		}
				//	}
				//	// completed this node so add as ancestors
				//	ancestors.insert(pNode->_graphID);
				//	return;
				//};
				/**
				* GetGraphTreeRoots (private)
				*
				* Iterate through the Nodes and look for nodes with no parents ( _outedges.size() = 0 )
				* 
				*/
				::std::unordered_set<GRAPHID> GetGraphTreeRoots()
				{        
					::std::unordered_set<GRAPHID> roots;

					::std::vector<Node*>::iterator nodeIT = _nodes.begin();
					while( nodeIT != _nodes.end() )
					{
						if( (*nodeIT)->_outedges.size() == 0)
						{
							roots.insert( (*nodeIT)->_graphID );
						}
						++nodeIT;
					}
					return ::std::move(roots);
				};
				/**
				* GetGraphTreeLeafs (private)
				*
				* DFS search of a tree from a given node. (usually root node)
				* 
				*/
				void GetGraphTreeLeafs(Node* pNode, ::std::unordered_set<GRAPHID> &leafs)
				{  
					// if this node has children (or dependants)
					if(pNode->_inedges.size() != 0)
					{
						::std::vector<Edge*>::iterator edgeIT = pNode->_inedges.begin();
						while( edgeIT != pNode->_inedges.end() )
						{
							GetGraphTreeLeafs( (*edgeIT)->_innode, leafs);
							++edgeIT;
						}
					}
					else
					{
						// no children (dependants) so add to leafs
						leafs.insert(pNode->_graphID);
					}

					return;
				};
				/**
				* GetGraphNodeAncestors (private)
				*
				* Bottom -> Up DFS search of a tree from a given node saving all nodes visited 
				* therefore saving all ansectors through to the root.
				* 
				*/
				void GetGraphNodeAncestors(Node* pNode, ::std::unordered_set<GRAPHID> &ancestors)
				{  
					// if this node has parents (is dependant on ...)
					if(pNode->_outedges.size() != 0)
					{
						::std::vector<Edge*>::iterator edgeIT = pNode->_outedges.begin();
						while( edgeIT != pNode->_outedges.end() )
						{
							GetGraphNodeAncestors( (*edgeIT)->_outnode, ancestors);
							++edgeIT;
						}
					}
					// completed this node so add as ancestors
					ancestors.insert(pNode->_graphID);
					return;
				};
				/**
				* GetGraphNodeChildren (private)
				*
				* DFS search of a tree from a given node. 
				* 
				*/
				void GetGraphNodeChildren(Node* pNode, ::std::unordered_set<GRAPHID> &children)
				{  
					// if this node has children (or dependants)
					if(pNode->_inedges.size() != 0)
					{
						::std::vector<Edge*>::iterator edgeIT = pNode->_inedges.begin();
						while( edgeIT != pNode->_inedges.end() )
						{
							GetGraphNodeChildren( (*edgeIT)->_innode, children);
							++edgeIT;
						}
					}
					children.insert(pNode->_graphID);
					return;
				};		

			private:
				ServiceDependencyGraph(const ServiceDependencyGraph&);
				ServiceDependencyGraph& operator=(const ServiceDependencyGraph&);

				Node* GetNodePtr(GRAPHID id)
				{
					::std::map<GRAPHID, Node*>::iterator it = _id_to_nodeptr.find(id);
					if( it != _id_to_nodeptr.end() )
					{
						return it->second;
					}
					return nullptr;
				};

				::std::vector<Node*>		_nodes;
				::std::vector<Edge*>		_edges;
				GRAPHID						_nextGraphID;
				::std::map<GRAPHID, Node*>	_id_to_nodeptr;
				::std::map<GRAPHID, Edge*>	_id_to_edgeptr;
			};  	
	}
}
	
