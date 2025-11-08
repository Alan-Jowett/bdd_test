# Pure Iterator DOT Template System

A modern, type-safe template system for generating Graphviz DOT diagrams using a pure iterator approach with optional, granular property methods automatically detected via SFINAE.

## Overview

This template system eliminates the need for separate data types by making iterators self-contained. The iterator IS the node, and child traversal returns new iterator instances. Individual DOT properties are provided through optional methods that are automatically detected at compile time, giving you complete control over what attributes are included.

## Files

- **`dot_graph_generator.hpp`** - Core template system with generic DOT generation
- **`dot_example.hpp`** - Example implementation for expression trees  
- **`dot_template_test.cpp`** - Simple test program demonstrating usage
- **`README_DOT_TEMPLATE.md`** - This documentation file

## Key Components

### DotConfig
Configuration structure for customizing graph appearance:
```cpp
dot_template::DotConfig config("MyGraph");
config.rankdir = "TB";           // Top-to-bottom layout
config.font_name = "Arial";      // Font family
config.default_node_shape = "circle";
config.show_node_ids = false;
```

### NodeProperties
Defines how individual nodes appear in the graph:
```cpp
dot_template::NodeProperties props;
props.label = "Node Label";
props.shape = "box";
props.fillcolor = "lightblue";
props.tooltip = "Additional info";
```

### EdgeProperties  
Defines how edges between nodes appear:
```cpp
dot_template::EdgeProperties edge_props;
edge_props.label = "L";          // Edge label
edge_props.style = "dashed";     // Line style
edge_props.color = "blue";       // Edge color
```

### DotNodeIterator<T>
Interface specification that your iterator should implement (no inheritance required - uses duck typing):

```cpp
class MyIterator {
public:
    // Required methods:
    std::vector<MyIterator> get_children() const;
    std::string get_node_id() const;
    
    // Optional node property methods (auto-detected via SFINAE):
    std::string get_label() const;
    std::string get_shape() const;           // circle, box, ellipse, diamond, etc.
    std::string get_style() const;           // filled, dashed, solid, etc.
    std::string get_fillcolor() const;       // lightblue, red, #FF0000, etc.
    std::string get_fontcolor() const;       // black, white, blue, etc.
    std::string get_tooltip() const;         // tooltip text
    
    // Optional edge property methods (auto-detected via SFINAE):
    std::string get_edge_label(const MyIterator& child, size_t index) const;
    std::string get_edge_style(const MyIterator& child, size_t index) const;     // solid, dashed, dotted, etc.
    std::string get_edge_color(const MyIterator& child, size_t index) const;     // black, blue, red, etc.
    std::string get_edge_fontcolor(const MyIterator& child, size_t index) const; // color for edge labels
    
    // Optional filtering:
    bool should_process() const; 
};
```

**Note**: Only `get_children()` and `get_node_id()` are required! All other methods are optional and automatically detected via SFINAE. The template composes the final DOT attributes from whatever property methods you provide.

## Usage Example

```cpp
#include "dot_graph_generator.hpp"

// 1. Define your iterator (no inheritance required)
class MyTreeIterator {
public:
    // Implement required methods using duck typing...
    dot_template::NodeProperties get_node_properties(const MyTree& data) const;
    std::vector<MyTree> get_children(const MyTree& data) const;
    dot_template::EdgeProperties get_edge_properties(const MyTree& parent, const MyTree& child, size_t index) const;
    std::string get_node_id(const MyTree& data) const;
    // Optional: bool should_process(const MyTree& data) const;
};

// 2. Generate the graph  
void generate_my_graph(const MyTree& root, std::ostream& out) {
    MyTreeIterator iterator;
    dot_template::DotConfig config("MyTreeGraph");
    dot_template::generate_dot_graph(root, iterator, out, config);
}
```

## Testing the Template

Compile and run the test program:
```bash
g++ -std=c++17 -I. dot_template_test.cpp -o dot_template_test
./dot_template_test
```

This will:
1. Create a sample tree structure
2. Generate DOT output to console
3. Write a `test_tree.dot` file
4. Show command to visualize with Graphviz

## Integration with Existing Code

The template system can easily replace existing DOT generation code:

### Before (manual DOT generation):
```cpp
void write_to_dot(const MyData& data, std::ostream& out) {
    out << "digraph MyGraph {\n";
    // Manual node and edge generation...
    out << "}\n";
}
```

### After (using duck typing template):
```cpp
class MyDataIterator {
public:
    // Implement required methods (no inheritance needed)...
    dot_template::NodeProperties get_node_properties(const MyData& data) const;
    std::vector<MyData> get_children(const MyData& data) const;
    dot_template::EdgeProperties get_edge_properties(const MyData& parent, const MyData& child, size_t index) const;
    std::string get_node_id(const MyData& data) const;
};

void write_to_dot(const MyData& data, std::ostream& out) {
    MyDataIterator iterator;
    dot_template::generate_dot_graph(data, iterator, out, "MyGraph");
}
```

## Benefits

1. **Code Reuse** - Common DOT structure logic is centralized
2. **Type Safety** - Template system prevents type mismatches  
3. **Flexibility** - Easy to customize appearance and behavior
4. **Duck Typing** - No inheritance required, works with any compatible class
5. **Maintainability** - Clear separation of concerns
6. **Extensibility** - Easy to add new data types or modify existing ones

## Advanced Features

### Custom Attributes
Add custom DOT attributes to nodes and edges:
```cpp
props.custom["penwidth"] = "2";
props.custom["fontsize"] = "14";
```

### Conditional Processing
Filter which nodes to include:
```cpp
bool should_process(const MyData& data) const override {
    return !data.is_hidden; // Skip hidden nodes
}
```

### Dynamic Styling
Change appearance based on data:
```cpp
NodeProperties get_node_properties(const MyData& data) const override {
    NodeProperties props;
    props.label = data.name;
    props.fillcolor = data.is_error ? "red" : "lightblue";
    return props;
}
```

## Requirements

- C++17 or later
- Standard library (no external dependencies)
- Graphviz for visualization (optional, for viewing generated DOT files)

## License

Part of the BDD Expression Converter project.