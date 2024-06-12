from copy import deepcopy
from typing import Dict, Generator, List, NamedTuple, Set, Tuple

from ..sv_helpers import SvInterface
from .endpoints import Endpoint
from .node import Node


class TargetInf(NamedTuple):
    """Named Tuple containing local information about targets"""
    ep: Endpoint
    parent: Node

class SourceInf(NamedTuple):
    """Named Tuple containing local information about sources"""
    ep: Endpoint
    parent: Node
    to: str

class ConIn(NamedTuple):
    """Information about incomming connection to a node."""
    designator: Tuple[str, str]
    direction: int
    first: bool

class BundledCon(NamedTuple):
    """Holds informations about one connection in a bundle."""
    source: str
    target: str
    source_idx: int


class RoutingHelper():
    """
    Helper to route signals between nodes (generaly sv modules).
    
    Provided with nodes, sources and targets it can create all the definition of the signals and bundle them in interfaces.

    It has the following limitations:
        - names have to be unique for each category, even across nodes.
    """

    def __init__(self) -> None:
        self._root_node = Node("root", "")
        self._nodes: Dict[str, Node] = {"root": deepcopy(self._root_node)}
        self._sources: Dict[str, SourceInf] = dict()
        self._targets: Dict[str, TargetInf] = dict()
        self._node_childs: Dict[str, List[str]] = {"root": []}

        self._bundles: Dict[Tuple[str,str], List[BundledCon]] = dict()
        self._local_connections: Dict[str, List[Tuple[str, str]]] = dict()

        self._source_full_sv: Dict[str, Dict[str, str]] = dict()
        self._target_full_sv: Dict[str, Dict[str, str]] = dict()
        self._target_full_sv_multi: Dict[str, Dict[str, List[str]]] = dict()

    def _add_node_child(self, parent: str, child: str):
        """
        Internal methode to add childs to a node.
        """
        self._node_childs[parent].append(str)
        self._node_childs[child] = []

    def register_node(self, node: Node):
        """
        Register a node in the helper.
        
        This is needed in order to add sources or targets to this node.

        :param Node node: the node to add.
        """
        if not isinstance(node, Node):
            raise TypeError("node is not an instance of Node.")

        if node.name in self._nodes:
            raise RuntimeError("A node with the same name is already registered.")
        
        if node.p_name == "":
            raise RuntimeError("It is not possible to add a new root node.") #because the code does not handle it.
        
        if not node.p_name in self._nodes:
            raise RuntimeError("Node added before it's parent.")

        self._nodes.update({node.name: deepcopy(node)})
        self._add_node_child(node.p_name, node.name)
        

    def add_source(self, parent: Node, name: str, ep: Endpoint, to: str = ""):
        """
        Add a source tied to a specific node. If `to` is not specified it will connect to the first available target.
        
        :param Node parent: The parent of this source
        :param str name: the name of this source
        :param Endpoint ep: The endpoint of this source
        :param str to: the name of the target the source should be connected
        """
        if not isinstance(parent, Node):
            raise TypeError("parent should be an instance of Node")
        if type(name) is not str or str == "":
            raise TypeError("name should be a non empty string")
        if not isinstance(ep, Endpoint):
            raise TypeError("ep should be of type Endpoint")
        if type(to) is not str:
            raise TypeError("to should be of type str")
        
        if parent.name not in self._nodes:
            raise RuntimeError("parent Node has to be added before usage")
        if name in self._sources:
            raise RuntimeError("a name can't be used twice as source name")
        
        self._sources.update({name: SourceInf(ep, parent, to)})

    def add_target(self, parent: Node, name: str, ep: Endpoint):
        """
        Add a target tied to a specific node.
        
        :param Node parent: The parent of this target
        :param str name: the name of this target
        :param Endpoint ep: The endpoint of this target
        """
        if not isinstance(parent, Node):
            raise TypeError("parent should be an instance of Node")
        if type(name) is not str or str == "":
            raise TypeError("name should be a non empty string")
        if not isinstance(ep, Endpoint):
            raise TypeError("ep should be of type Endpoint")
        
        if parent.name not in self._nodes:
            raise RuntimeError("parent Node has to be added before usage")
        
        if name in self._targets:
            raise RuntimeError("a name can't be used twice as target name")
        
        self._targets.update({name: TargetInf(ep, parent)})

    def get_parent(self, node: str) -> str:
        """
        Get the parent of a specific node by name
        
        :param str node: The name of the node
        :return: the parent name
        :rtype: str
        """
        if type(node) is not str:
            raise TypeError("node should be of type string")
        if not node in self._nodes:
            raise RuntimeError(f"node {node} not registerd")
        return self._nodes[node].p_name

    def _parent_iter(self, node: str) -> Generator[str, None, None]:
        """
        Internal methode to iterate over all parent of a node from the node to the root."""
        while True:
            if node in self._nodes:
                yield node
            else:
                return
            
            node = self.get_parent(node)

    def route(self):
        """
        Do all the calculation to connect all targets and sources together.

        Targets and sources should be added before this is called.
        """
        pairs: List[Tuple[str, str]] = []
        sources_used: Set[str] = set()
        targets_used: Set[str] = set()

        # route source with named target
        for s, inf in self._sources.items():
            if inf.to != "":
                if inf.to in self._targets:
                    if not inf.to in targets_used:

                        pairs.append((s, inf.to))
                        sources_used.add(s)
                        full = self._targets[inf.to].ep.use_target(s)
                        if full:
                            targets_used.add(inf.to)
                    else:
                        raise RuntimeError(f"Unable to route {s} to {inf.to}: {inf.to} is already used or all ports are used.")

                else:
                    raise RuntimeError(f"Unable to route {s} to {inf.to}: {inf.to} is not provided to routing helper.")
        
        # route source without named target
        for s, s_inf in self._sources.items():

            if s_inf.to == "":
                target = None
                for t, t_inf in self._targets.items():
                    if t in targets_used:
                        continue

                    if isinstance(t_inf.ep, type(s_inf.ep)):
                        target = t
                        break

                if target is not None:
                    pairs.append((s, target))
                    sources_used.add(s)
                    full = self._targets[target].ep.use_target(s)
                    if full:
                        targets_used.add(target)
                else:
                    raise RuntimeError(f"Unable to route {s}: no tagets are available or all are used.")
        

        # bundle connections together.
        self._bundles = dict()
        self._local_connections = dict()

        node_to_connect: Set[Tuple[str, str]] = set()
        for s, t in pairs:
            s_inf = self._sources[s]
            t_inf = self._targets[t]

            if t_inf.parent == s_inf.parent:
                self._local_connections.setdefault(t_inf.parent.name, []).append((s, t))

            else:
                p_set = tuple(sorted([s_inf.parent.name, t_inf.parent.name]))
                node_to_connect.add(p_set)
                direc = 0 if p_set[0] == s_inf.parent.name else 1
                self._bundles.setdefault(p_set, []).append(BundledCon(s, t, direc))
        

        self._connection_roots: Dict[str, List[Tuple[str, str]]] = dict()
        self._connection_in: Dict[str, List[ConIn]] = dict()

        for i in node_to_connect:
            path0 = list(self._parent_iter(i[0]))[::-1]
            path1 = list(self._parent_iter(i[1]))[::-1]

            j = 0
            for p1, p2 in zip(path0, path1):
                if p1 != p2:
                    break
                j += 1
            
            if not path0[j-1] in self._connection_roots:
                self._connection_roots[path0[j-1]] = []
            self._connection_roots[path0[j-1]].append(i)
            
            first = True
            for n in path0[j:]:
                if not n in self._connection_in:
                    self._connection_in[n] = []
                self._connection_in[n].append(ConIn(i, 0, first))
                first = False
            
            first = True
            for n in path1[j:]:
                if not n in self._connection_in:
                    self._connection_in[n] = []
                self._connection_in[n].append(ConIn(i, 1, first))
                first = False
        
        self._generate_local_names()

    @staticmethod
    def bundle_name(n_name: Tuple[str, str]) -> str:
        """
        Get the name of of an interface connecting to nodes
        
        :param Tuple[str,str] n_name: The name of both nodes
        :return: the name of the bundle
        :rtype: str
        """
        return f"bundle__{n_name[0]}__{n_name[1]}__if"
    
    @staticmethod
    def bundle_type_name(n_name: Tuple[str, str]) -> str:
        """
        Get the type name of of an interface connecting to nodes
        
        :param Tuple[str,str] n_name: The name of both nodes
        :return: the type name of the bundle
        :rtype: str
        """
        return f"if_bundle__{n_name[0]}__{n_name[1]}"

    def _generate_local_names(self):
        """
        Internal methode to generate the sv names for each target and source.
        """
        for n_name, l in self._local_connections.items():
            d_s: Dict[str,str] = dict()
            d_t: Dict[str,str] = dict()
            d_t_multi: Dict[str,List[str]] = dict()
            for s, t in l:
                d_s.update({s: s})
                if self._targets[t].ep.single_use():
                    d_t.update({t: s})
                else:
                    d_t_multi.setdefault(t, []).append(s)

            self._source_full_sv[n_name] = d_s
            self._target_full_sv[n_name] = d_t
            self._target_full_sv_multi[n_name] = d_t_multi

        for n_name, l in self._bundles.items():
            for con in l:
                s_parent = n_name[con.source_idx]
                t_parent = n_name[1-con.source_idx]

                self._source_full_sv.setdefault(
                    s_parent, {}
                ).update(
                    {con.source: self.bundle_name(n_name)+"."+con.source}
                )
                
                if self._targets[con.target].ep.single_use():
                    self._target_full_sv.setdefault(
                        t_parent, {}
                    ).update(
                        {con.target: self.bundle_name(n_name)+"."+con.source}
                    )
                else:
                    self._target_full_sv_multi.setdefault(
                        t_parent, {}
                    ).setdefault(
                        con.target, []
                    ).append(
                        self.bundle_name(n_name)+"."+con.source
                    )


            


    def use_source_as_sv(self, name: str, p_node: Node) -> str:
        """
        Get the sv code to use a source in sv.
        
        :param str name: the name of the source
        :param Node p_node: the parent node
        :return: the sv code to use.
        :rtype: str
        """
        if type(name) is not str:
            raise TypeError("name should be of type str")
        if not isinstance(p_node, Node):
            raise TypeError("p_node should be an instance of Node")
        if not p_node.name in self._nodes:
            raise RuntimeError(f"node {p_node.name} is not registered")
        return self._source_full_sv[p_node.name][name]
    
    def use_target_as_sv(self, name: str, p_node: Node) -> str:
        """
        Get the sv code to use a target in sv.

        If the target is not connected, some endpoints do specify an empty string for outgoing signals.
        In some cases this has to be checked, like when an assignment is used.
        
        :param str name: the name of the target
        :param Node p_node: the parent node
        :return: the sv code to use.
        :rtype: str
        """
        if type(name) is not str:
            raise TypeError("name should be of type str")
        if not isinstance(p_node, Node):
            raise TypeError("p_node should be an instance of Node")
        if not p_node.name in self._nodes:
            raise RuntimeError(f"node {p_node.name} is not registered")
        
        if self._targets[name].ep.nc:
            return self._targets[name].ep.target_nc()
        
        return self._target_full_sv[p_node.name][name]
    
    def use_target_as_sv_multi(self, name: str, p_node: Node) -> List[str]:
        """
        Get the sv code to use a target in sv, with multiple sources.

        If the target is not connected, some endpoints do specify an empty string for outgoing signals.
        In some cases this has to be checked, like when an assignment is used.
        
        :param str name: the name of the target
        :param Node p_node: the parent node
        :return: the sv code to use.
        :rtype: List[str]
        """
        if type(name) is not str:
            raise TypeError("name should be of type str")
        if not isinstance(p_node, Node):
            raise TypeError("p_node should be an instance of Node")
        if not p_node.name in self._nodes:
            raise RuntimeError(f"node {p_node.name} is not registered")
        
        if self._targets[name].ep.nc:
            return self._targets[name].ep.target_nc()

        return self._target_full_sv_multi[p_node.name][name]
    
    def use_target_as_sv_array_multi(self, name: str, p_node: Node, size: int, default: str = "1'b0") -> List[str]:
        """
        Get the sv code to use a target in sv, with multiple sources in an sv array.

        If the target is not connected, some endpoints do specify an empty string for outgoing signals.
        In some cases this has to be checked, like when an assignment is used.
        
        :param str name: the name of the target
        :param Node p_node: the parent node
        :return: the sv code to use.
        :rtype: List[str]
        """
        if type(name) is not str:
            raise TypeError("name should be of type str")
        if not isinstance(p_node, Node):
            raise TypeError("p_node should be an instance of Node")
        if not p_node.name in self._nodes:
            raise RuntimeError(f"node {p_node.name} is not registered")
        
        try:
            names = self._target_full_sv_multi[p_node.name][name]
        except KeyError:
            names = []
        
        names += [default for _ in range(size - len(names))]
        return f"{{{','.join(names[::-1])}}}"
    
    def get_target_ep_copy(self, name: str) -> Endpoint:
        """
        Get an endpoint of a target by name
        
        :param str name: the target name
        :return: an endpoint
        :rtype: Endpoint
        """
        return deepcopy(self._targets[name].ep)
    
    def get_source_ep_copy(self, name: str) -> Endpoint:
        """
        Get an endpoint of a source by name
        
        :param str name: the source name
        :return: an endpoint
        :rtype: Endpoint
        """
        return deepcopy(self._sources[name].ep)
        


    def get_intf_sv_helpers(self) -> Generator[SvInterface, None, None]:
        """
        Makes the sv code for all interfaces.

        :return: a generator of SvInterface over each interface that is required.
        :rtype: Generator[SvInterface, None, None]
        """
        for n_name, l_con in self._bundles.items():
            helper = SvInterface(self.bundle_type_name(n_name))
            for connection in l_con:
                source_idx = connection.source_idx
                target_idx = 1-connection.source_idx

                source_name = n_name[source_idx]
                target_name = n_name[target_idx]

                source_ep = self._sources[connection.source].ep

                ports = [
                    (source_name, source_ep.source_direction),
                    (target_name, source_ep.target_direction),
                ]

                helper.add_signal(source_ep.t, connection.source, ports)
            
            yield helper

    def get_node_ports(self, node: Node) -> str:
        """
        Makes the sv code for the definition of a module
        
        :param Node node: The node linked to the module
        :return: the sv code with a trailing `,`
        :rtype: str
        """
        out = ""
        for con_in in self._connection_in[node.name]:
            type_name = self.bundle_type_name(con_in.designator)
            name = self.bundle_name(con_in.designator)
            port_name = con_in.designator[con_in.direction]

            out += f"{type_name}.{port_name} {name},"
        return out


    def get_node_local_signals(self, node: Node) -> str:
        """
        Makes the sv code for the definition of local signals to a module
        
        :param Node node: The node linked to the module
        :return: the sv code
        :rtype: str
        """
        out = ""
        if node.name in self._connection_roots:
            for n_name in self._connection_roots[node.name]:
                type_name = self.bundle_type_name(n_name)
                name = self.bundle_name(n_name)

                out += f"{type_name} {name};"
        
        if node.name in self._local_connections:
            for s, t in self._local_connections[node.name]:
                type_name = self._sources[s].ep.t
                name = s
                out += f"{type_name} {name};"
        return out

    def get_instantiation_signals(self, node: Node) -> str:
        """
        Makes the sv code for the instantiation of a module connections.
        
        :param Node node: The node linked to the module
        :return: the sv code with a trailing `,`
        :rtype: str
        """
        out = ""
        for con_in in self._connection_in[node.name]:
            name = self.bundle_name(con_in.designator)
            out += f".{name}"

            if con_in.first:
                port_name = con_in.designator[con_in.direction]
                out += f"({name}.{port_name})"
            
            out += ","
        
        return out


    def get_root_node(self) -> Node:
        """
        Get the root node of the helpers tree
        
        :return: the root node
        :rtype: Node
        """
        return self._root_node