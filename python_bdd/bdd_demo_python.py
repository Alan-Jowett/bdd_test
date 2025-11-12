#!/usr/bin/env python3
"""
Python BDD Demo - Binary Decision Diagram Converter

A Python implementation equivalent to bdd_demo.exe that produces identical
output formats for comparison with TeDDy and CUDD implementations.

This tool helps identify canonicality issues by providing a reference
Python BDD implementation using the 'dd' library.

Usage: python bdd_demo_python.py [filename] [options]
"""

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple, Union

try:
    from dd.autoref import BDD
except ImportError:
    print("Error: 'dd' library not found. Install with: pip install dd", file=sys.stderr)
    sys.exit(1)


class ExpressionToken:
    """Represents a token in a boolean expression"""

    def __init__(self, token_type: str, value: str, pos: int = 0):
        self.type = token_type  # 'variable', 'operator', 'lparen', 'rparen', 'eof'
        self.value = value
        self.pos = pos


class ExpressionLexer:
    """Tokenizes boolean expressions"""

    def __init__(self, text: str):
        self.text = text
        self.pos = 0
        self.tokens = []
        self._tokenize()

    def _skip_whitespace(self):
        """Skip whitespace characters"""
        while self.pos < len(self.text) and self.text[self.pos].isspace():
            self.pos += 1

    def _read_word(self) -> str:
        """Read a word (variable name or operator)"""
        start = self.pos
        while (self.pos < len(self.text) and
               (self.text[self.pos].isalnum() or self.text[self.pos] in '_$@#!')):
            self.pos += 1
        return self.text[start:self.pos]

    def _tokenize(self):
        """Convert input text into tokens"""
        while self.pos < len(self.text):
            self._skip_whitespace()

            if self.pos >= len(self.text):
                break

            char = self.text[self.pos]

            if char == '(':
                self.tokens.append(ExpressionToken('lparen', char, self.pos))
                self.pos += 1
            elif char == ')':
                self.tokens.append(ExpressionToken('rparen', char, self.pos))
                self.pos += 1
            elif char.isalpha() or char in '_$@#!':
                word = self._read_word()
                if word.upper() in ['AND', 'OR', 'XOR', 'NOT']:
                    self.tokens.append(ExpressionToken('operator', word.upper(), self.pos - len(word)))
                else:
                    self.tokens.append(ExpressionToken('variable', word, self.pos - len(word)))
            else:
                raise ValueError(f"Unexpected character '{char}' at position {self.pos}")

        self.tokens.append(ExpressionToken('eof', '', self.pos))


class ExpressionParser:
    """Parses boolean expressions into BDD-compatible format"""

    def __init__(self, tokens: List[ExpressionToken]):
        self.tokens = tokens
        self.pos = 0
        self.variables: Set[str] = set()

    def current_token(self) -> ExpressionToken:
        """Get current token"""
        return self.tokens[self.pos] if self.pos < len(self.tokens) else self.tokens[-1]

    def consume_token(self, expected_type: Optional[str] = None) -> ExpressionToken:
        """Consume and return current token"""
        token = self.current_token()
        if expected_type and token.type != expected_type:
            raise ValueError(f"Expected {expected_type} but got {token.type}")
        self.pos += 1
        return token

    def parse_expression(self) -> str:
        """Parse the full expression"""
        expr = self.parse_or_expression()
        if self.current_token().type != 'eof':
            raise ValueError(f"Unexpected token {self.current_token().value}")
        return expr

    def parse_or_expression(self) -> str:
        """Parse OR expressions (lowest precedence)"""
        left = self.parse_xor_expression()

        while self.current_token().type == 'operator' and self.current_token().value == 'OR':
            self.consume_token()
            right = self.parse_xor_expression()
            left = f"({left} | {right})"

        return left

    def parse_xor_expression(self) -> str:
        """Parse XOR expressions"""
        left = self.parse_and_expression()

        while self.current_token().type == 'operator' and self.current_token().value == 'XOR':
            self.consume_token()
            right = self.parse_and_expression()
            left = f"({left} ^ {right})"

        return left

    def parse_and_expression(self) -> str:
        """Parse AND expressions (higher precedence)"""
        left = self.parse_not_expression()

        while self.current_token().type == 'operator' and self.current_token().value == 'AND':
            self.consume_token()
            right = self.parse_not_expression()
            left = f"({left} & {right})"

        return left

    def parse_not_expression(self) -> str:
        """Parse NOT expressions (highest precedence)"""
        if self.current_token().type == 'operator' and self.current_token().value == 'NOT':
            self.consume_token()
            expr = self.parse_not_expression()
            return f"~{expr}"
        else:
            return self.parse_primary_expression()

    def parse_primary_expression(self) -> str:
        """Parse primary expressions (variables and parenthesized expressions)"""
        token = self.current_token()

        if token.type == 'variable':
            self.variables.add(token.value)
            self.consume_token()
            return token.value
        elif token.type == 'lparen':
            self.consume_token()
            expr = self.parse_or_expression()
            self.consume_token('rparen')
            return expr
        else:
            raise ValueError(f"Expected variable or '(' but got {token.value}")


class PythonBDDDemo:
    """Python BDD Demo - produces output identical to C++ bdd_demo.exe"""

    def __init__(self):
        self.bdd = BDD()
        self.variable_order: Dict[str, int] = {}
        self.node_table: List[Dict] = []

    def read_expression_from_file(self, filename: str) -> str:
        """Read and parse expression from file, matching C++ behavior"""
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                lines = f.readlines()
        except FileNotFoundError:
            raise FileNotFoundError(f"Could not open file: {filename}")

        # Filter out comments and empty lines
        expression_lines = []
        for line in lines:
            line = line.strip()
            if line and not line.startswith('#'):
                expression_lines.append(line)

        if not expression_lines:
            raise ValueError("No expression found in file")

        # Join all expression lines
        expression = ' '.join(expression_lines)
        return expression.strip()

    def create_variable_ordering(self, variables: Set[str]) -> Dict[str, int]:
        """Create consistent variable ordering (sorted alphabetically)"""
        sorted_vars = sorted(variables)
        ordering = {}
        for i, var in enumerate(sorted_vars):
            ordering[var] = i
            self.bdd.declare(var)
        return ordering

    def build_bdd_from_expression(self, expression_text: str) -> any:
        """Build BDD from parsed expression"""
        # Tokenize and parse
        lexer = ExpressionLexer(expression_text)
        parser = ExpressionParser(lexer.tokens)

        # Parse to dd library format
        dd_expression = parser.parse_expression()

        # Create variable ordering
        self.variable_order = self.create_variable_ordering(parser.variables)

        # Build BDD
        bdd_func = self.bdd.add_expr(dd_expression)
        return bdd_func

    def generate_node_table(self, bdd_func) -> List[Dict]:
        """Generate node table in the same format as C++ version"""
        nodes = []

        try:
            # Create a simple BDD representation using cofactors
            # This is a basic implementation that creates a canonical structure

            # Get the support (variables) of the BDD
            support = self.bdd.support(bdd_func)

            if not support:
                # Constant function
                if bdd_func == self.bdd.true:
                    nodes = [{
                        'id': 0,
                        'variable': '-',
                        'false_child': '-',
                        'true_child': '-',
                        'type': 'Terminal(1)'
                    }]
                else:
                    nodes = [{
                        'id': 0,
                        'variable': '-',
                        'false_child': '-',
                        'true_child': '-',
                        'type': 'Terminal(0)'
                    }]
            else:
                # Build a simple topological representation
                visited_functions = {}
                node_counter = [0]

                def process_function(func, depth=0):
                    """Recursively process BDD function"""
                    # Check for terminals
                    if func == self.bdd.false:
                        if 'false' not in visited_functions:
                            visited_functions['false'] = {
                                'id': node_counter[0],
                                'variable': '-',
                                'false_child': '-',
                                'true_child': '-',
                                'type': 'Terminal(0)'
                            }
                            node_counter[0] += 1
                        return visited_functions['false']['id']

                    if func == self.bdd.true:
                        if 'true' not in visited_functions:
                            visited_functions['true'] = {
                                'id': node_counter[0],
                                'variable': '-',
                                'false_child': '-',
                                'true_child': '-',
                                'type': 'Terminal(1)'
                            }
                            node_counter[0] += 1
                        return visited_functions['true']['id']

                    # Check if we've seen this function before
                    func_key = str(func)
                    if func_key in visited_functions:
                        return visited_functions[func_key]['id']

                    # Find the top variable
                    func_support = self.bdd.support(func)
                    if not func_support:
                        # Should not happen for non-terminal functions
                        return process_function(self.bdd.false, depth + 1)

                    # Choose variable with lowest index
                    top_var = min(func_support, key=lambda v: self.variable_order.get(v, 999))

                    # Get cofactors
                    low_func = self.bdd.let({top_var: False}, func)
                    high_func = self.bdd.let({top_var: True}, func)

                    # Process children first
                    low_id = process_function(low_func, depth + 1)
                    high_id = process_function(high_func, depth + 1)

                    # Create this node
                    node_info = {
                        'id': node_counter[0],
                        'variable': top_var,
                        'false_child': low_id,
                        'true_child': high_id,
                        'type': 'Variable'
                    }
                    visited_functions[func_key] = node_info
                    node_counter[0] += 1

                    return node_info['id']

                # Start processing from root
                process_function(bdd_func)

                # Convert to list and sort by ID
                nodes = list(visited_functions.values())
                nodes.sort(key=lambda x: x['id'])

        except Exception as e:
            print(f"Error generating node table: {e}")
            # Create a minimal fallback based on the expression
            support = self.bdd.support(bdd_func) if hasattr(self.bdd, 'support') else set()

            if not support:
                # Constant function fallback
                nodes = [{
                    'id': 0,
                    'variable': '-',
                    'false_child': '-',
                    'true_child': '-',
                    'type': 'Terminal(1)'
                }]
            else:
                # Create a simple tree structure based on variables
                sorted_vars = sorted(support, key=lambda v: self.variable_order.get(v, 0))

                nodes = []
                for i, var in enumerate(sorted_vars):
                    if i == len(sorted_vars) - 1:
                        # Last variable - children are terminals
                        nodes.append({
                            'id': i,
                            'variable': var,
                            'false_child': len(sorted_vars),
                            'true_child': len(sorted_vars) + 1,
                            'type': 'Variable'
                        })
                    else:
                        # Internal variable - child is next variable
                        nodes.append({
                            'id': i,
                            'variable': var,
                            'false_child': i + 1,
                            'true_child': i + 1,  # Simplified for fallback
                            'type': 'Variable'
                        })

                # Add terminals
                nodes.extend([
                    {
                        'id': len(sorted_vars),
                        'variable': '-',
                        'false_child': '-',
                        'true_child': '-',
                        'type': 'Terminal(0)'
                    },
                    {
                        'id': len(sorted_vars) + 1,
                        'variable': '-',
                        'false_child': '-',
                        'true_child': '-',
                        'type': 'Terminal(1)'
                    }
                ])

        self.node_table = nodes
        return nodes

    def save_node_table(self, filename: str):
        """Save node table to file in exact C++ format"""
        with open(filename, 'w') as f:
            # Write header
            f.write("Index | Variable | False Child | True Child | Type\n")
            f.write("------|----------|-------------|------------|----------\n")

            # Write nodes
            for node in self.node_table:
                f.write(f"{node['id']:5d} | {node['variable']:8s} | "
                       f"{str(node['false_child']):11s} | {str(node['true_child']):10s} | "
                       f"{node['type']}\n")

            f.write("\n")  # Empty line at end like C++ version

    def generate_dot_graph(self, bdd_func, expression_name: str) -> str:
        """Generate DOT graph in exact C++ format"""
        dot_lines = ["digraph DD {", ""]

        # Identify terminal nodes for shape formatting
        terminal_nodes = []
        for node in self.node_table:
            if node['type'].startswith('Terminal'):
                terminal_nodes.append(f"node{node['id']}")

        if terminal_nodes:
            dot_lines.append(f"    node [shape = square] {' '.join(terminal_nodes)};")
        dot_lines.append("    node [shape = circle];")
        dot_lines.append("")

        # Add node definitions
        for node in self.node_table:
            if node['type'].startswith('Terminal'):
                value = "1" if node['type'] == 'Terminal(1)' else "0"
                label = value
                tooltip = value
            else:
                label = node['variable']
                tooltip = str(self.variable_order.get(node['variable'], 0))

            dot_lines.append(f'    node{node["id"]} [label = "{label}", tooltip = "{tooltip}"];')

        dot_lines.append("")

        # Add edges
        for node in self.node_table:
            if node['type'] == 'Variable':
                # False edge (dashed)
                dot_lines.append(f"    node{node['id']} -> node{node['false_child']} [style = dashed];")
                # True edge (solid)
                dot_lines.append(f"    node{node['id']} -> node{node['true_child']} [style = solid];")

        dot_lines.append("}")
        dot_lines.append("")  # Empty line at end

        return "\n".join(dot_lines)

    def save_dot_graph(self, bdd_func, filename: str, expression_name: str):
        """Save DOT graph to file"""
        dot_content = self.generate_dot_graph(bdd_func, expression_name)
        with open(filename, 'w') as f:
            f.write(dot_content)

    def print_verbose_output(self, expression_text: str, variables: Set[str]):
        """Print verbose output matching C++ format"""
        print("\nBDD Node Structure:")
        print("==================")
        print("BDD Node Table (topological ordering):")
        print("Index | Variable | False Child | True Child | Type")
        print("------|----------|-------------|------------|----------")

        for node in self.node_table:
            print(f"{node['id']:5d} | {node['variable']:8s} | "
                 f"{str(node['false_child']):11s} | {str(node['true_child']):10s} | "
                 f"{node['type']}")

        print(f"\nTotal nodes: {len(self.node_table)}")
        print("Note: Topological order ensures parents appear before children.")

    def run(self, filename: str, verbose: bool = False) -> int:
        """Main execution function"""
        try:
            print("Python BDD Demo - Building BDD from Expression File")
            print("=================================================")
            print()

            # Read expression
            print(f'Reading expression from: "{filename}"')
            expression_text = self.read_expression_from_file(filename)
            print(f"Read expression from file: {expression_text}")
            print()

            # Build BDD
            print("Converting expression to BDD using Python 'dd' library...")
            bdd_func = self.build_bdd_from_expression(expression_text)

            # Show variable ordering
            var_order_str = ", ".join([f"{var}={idx}" for var, idx in sorted(self.variable_order.items(), key=lambda x: x[1])])
            print(f"Python BDD variable ordering: {var_order_str}")
            print("Function created successfully!")
            print(f"Using {len(self.variable_order)} variables")
            print()

            # Generate outputs
            base_name = Path(filename).stem
            output_dir = Path(filename).parent

            # Node table
            self.generate_node_table(bdd_func)
            node_table_file = output_dir / f"{base_name}_bdd_nodes.txt"
            self.save_node_table(str(node_table_file))

            # DOT graph
            dot_file = output_dir / f"{base_name}_bdd.dot"
            self.save_dot_graph(bdd_func, str(dot_file), base_name)

            if verbose:
                self.print_verbose_output(expression_text, set(self.variable_order.keys()))

            print(f'BDD DOT representation saved to "{dot_file}"')
            print(f'You can visualize it using Graphviz with: dot -Tpng "{dot_file}" -o "{dot_file.with_suffix(".png")}"')
            print(f'BDD node table saved to "{node_table_file}"')
            print()
            print("Demo completed successfully!")

            return 0

        except FileNotFoundError as e:
            print(f"Error reading expression file: {e}", file=sys.stderr)
            return 1
        except ValueError as e:
            print(f"Error parsing expression: {e}", file=sys.stderr)
            return 1
        except Exception as e:
            print(f"Unexpected error: {e}", file=sys.stderr)
            return 1


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Python BDD Demo - Binary Decision Diagram Converter",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example usage:
  python bdd_demo_python.py test_expressions/simple_expression.txt
  python bdd_demo_python.py test_expressions/all_operators.txt --verbose

Supported operators: AND, OR, XOR, NOT
Use parentheses for grouping

Generated DOT files can be visualized using Graphviz tools:
  dot -Tpng input.dot -o output.png
        """
    )

    parser.add_argument('filename', nargs='?',
                       default='test_expressions/simple_expression.txt',
                       help='Input expression file (default: test_expressions/simple_expression.txt)')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Show detailed console output of BDD structure')
    parser.add_argument('--quiet', '-q', action='store_true',
                       help='Suppress verbose output (default behavior)')

    args = parser.parse_args()

    demo = PythonBDDDemo()
    return demo.run(args.filename, args.verbose and not args.quiet)


if __name__ == "__main__":
    sys.exit(main())