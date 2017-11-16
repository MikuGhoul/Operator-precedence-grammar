#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <set>

class Context_Free {
public:
	Context_Free() = default;
	Context_Free(char left, std::string right) : _context_left(left), _context_right(right) {}
	inline void print_test() const { std::cout << _context_left << std::endl << _context_right << std::endl; }
	inline char get_left() const { return _context_left; }
	inline std::string get_right() const { return _context_right; }
private:
	char _context_left;
	std::string _context_right;
};

// 返回一个[保存每一行文法的(每个候选式的指针)的]动态数组
std::vector<Context_Free*> tiny_lexical(std::string _context_line)
{
	std::vector<Context_Free*> context_vec;
	char _context_left = *_context_line.begin();	// 每一行的文法的左部一定相同
	size_t same_left = 1;
	for (auto x = _context_line.begin() + 3; x != _context_line.end(); ++x) {
		if (*x == '|') ++same_left;
	}
	std::vector<std::string> right_container;	// 文法的右部可能有多个候选式，这里用的string的vector存储
	// 反正我自己定义dsl，可以写个文档规定每行的文法输入格式为`E->E+T|T`, 非终结符必须大写，终结符必须小写啥的
	std::string::iterator flag = _context_line.begin() + 3; 
	if (same_left != 1) {
		for (auto x = _context_line.begin() + 3; x != _context_line.end(); ++x) {
			if (*x == '|') {
				right_container.push_back(std::string(flag, x));
				flag = x + 1;
			}
			else if (x == _context_line.end() - 1){
				right_container.push_back(std::string(flag, _context_line.end()));
			}
		}
	}
	else {
		right_container.push_back(std::string(flag, _context_line.end()));
	}
	for (size_t i = 0; i < same_left; ++i) {
		context_vec.push_back(new Context_Free(_context_left, right_container[i]));
	}
	return context_vec;
}

std::string init_flag(std::vector<std::vector<bool>> &_flag, const std::vector<std::vector<Context_Free*>> &_all_grammer)
{
	size_t grammer_num = _all_grammer.size();
	// 这里遍历出非终结符的个数，代码不是很多，就不抽象出来了
	std::string vt_num;
	std::map<char, bool> read_one_time;	// 保证只读一遍, 建立终结符到bool的映射
	for (const auto &_x : _all_grammer)
		for (const auto &_y : _x) {
			std::string _right_value = _y->get_right();
			for (auto _z = _right_value.begin(); _z != _right_value.end(); ++_z) {
				if ((*_z >= 'A' && *_z <= 'Z') || *_z == '|')	// 只读非终结符
					continue;
				if (read_one_time[*_z])
					continue;
				vt_num.push_back(*_z);
				read_one_time[*_z] = true;
			}
		}
	for (size_t i = 0; i < grammer_num; ++i) {
		std::vector<bool> temp_vec(vt_num.size(), false);
		_flag.push_back(temp_vec);
	}
	return vt_num;
}

void insert_operate(char _left_value, char _right_value, std::stack<std::vector<char>> &_temp_stack, std::vector<std::vector<bool>> &_flag, std::vector<std::map<char, size_t>> _vn_vt_index)
{
	size_t left_index = _vn_vt_index[0][_left_value];
	size_t right_index = _vn_vt_index[1][_right_value];
	if (!_flag[left_index][right_index]) {
		_flag[left_index][right_index] = true;
		std::vector<char> temp_vec;
		temp_vec.push_back(_left_value);
		temp_vec.push_back(_right_value);
		_temp_stack.push(temp_vec);
	}
	
}

// 这里_flag就不取引用了，因为还要计算vn，flag原值不能改，这里直接用vector的拷贝构造一个_flag
std::vector<std::vector<bool>> calc_last_vt(std::vector<std::vector<bool>> _flag, const std::vector<std::vector<Context_Free*>> &_all_grammer, 
	const std::set<char> &_vt_set, std::vector<std::map<char, size_t>> _vn_vt_index)
{
	std::stack<std::vector<char>> temp_stack;
	for (const auto &_x : _all_grammer)
		for (const auto &_y : _x) {
			std::string _right_value = _y->get_right();
			char _right_last = *(_right_value.end() - 1);
			char _right_second_last = 0;		
			if (_y->get_right().size() > 1)							// 针对右侧只有一个符号的情况
				_right_second_last = *(_right_value.end() - 2);
			if (_vt_set.find(_right_last) != _vt_set.end())			// 这里用set优化可以快一点
				insert_operate(_y->get_left(), *_vt_set.find(_right_last), temp_stack, _flag, _vn_vt_index);
			else if (_vt_set.find(_right_second_last) != _vt_set.end() && _right_last <= 'Z' && _right_last >= 'A')
				insert_operate(_y->get_left(), *_vt_set.find(_right_second_last), temp_stack, _flag, _vn_vt_index);
		}
	while (!temp_stack.empty()) {
		std::vector<char> temp_vec = temp_stack.top();
		temp_stack.pop();
		for (const auto &_x : _all_grammer)
			for (const auto &_y : _x) {
				char left_value = _y->get_left();
				std::string right_value = _y->get_right();
				if (*(right_value.end() - 1) == temp_vec[0])
					insert_operate(left_value, temp_vec[1], temp_stack, _flag, _vn_vt_index);
			}
	}
	return _flag;
}

std::vector<std::vector<bool>> calc_first_vt(std::vector<std::vector<bool>> _flag, const std::vector<std::vector<Context_Free*>> &_all_grammer, 
	const std::set<char> &_vt_set, std::vector<std::map<char, size_t>> _vn_vt_index)
{
	std::stack<std::vector<char>> temp_stack;
	for (const auto &_x : _all_grammer)
		for (const auto &_y : _x) {
			std::string _right_value = _y->get_right();
			char _right_first = *(_right_value.begin());
			char _right_second_first = 0;
			if (_y->get_right().size() > 1)
				_right_second_first = *(_right_value.begin() + 1);
			if (_vt_set.find(_right_first) != _vt_set.end())		
				insert_operate(_y->get_left(), *_vt_set.find(_right_first), temp_stack, _flag, _vn_vt_index);
			else if (_vt_set.find(_right_second_first) != _vt_set.end() && _right_first <= 'Z' && _right_first >= 'A')
				insert_operate(_y->get_left(), *_vt_set.find(_right_second_first), temp_stack, _flag, _vn_vt_index);
		}
	while (!temp_stack.empty()) {
		std::vector<char> temp_vec = temp_stack.top();
		temp_stack.pop();
		for (const auto &_x : _all_grammer)
			for (const auto &_y : _x) {
				char left_value = _y->get_left();
				std::string right_value = _y->get_right();
				if (*(right_value.begin()) == temp_vec[0])
					insert_operate(left_value, temp_vec[1], temp_stack, _flag, _vn_vt_index);
			}
	}
	return _flag;
}

// relationship_table有四种状态: `<` `>` `=` `0`
std::vector<std::vector<char>> init_relationship_table(const std::string &_vt_container)
{
	std::vector<std::vector<char>> relationship_table;
	for (const auto &_x : _vt_container) {
		std::vector<char> each_row_table;
		for (const auto &_y : _vt_container) {
			each_row_table.push_back(0);
		}
		relationship_table.push_back(each_row_table);
	}
	return relationship_table;
}

std::vector<std::vector<char>> calc_relationship_table(std::vector<std::vector<bool>> _last_vt, std::vector<std::vector<bool>> _first_vt, const std::vector<std::vector<Context_Free*>> &_all_grammer, 
	const std::set<char> &_vt_set, std::vector<std::map<char, size_t>> _vn_vt_index, std::vector<std::vector<char>> &relationship_table, const std::string &_vt_container)
{
	for (const auto &_x : _all_grammer)
		for (const auto &_y : _x) {
			std::string _right_value = _y->get_right();
			for (size_t i = 0; i < _right_value.size() - 1; ++i) {
				// a, b 优先级相同
				if (_vt_set.find(_right_value[i]) != _vt_set.end() && _vt_set.find(_right_value[i + 1]) != _vt_set.end()) {
					size_t i_1 = _vn_vt_index[1][_right_value[i]];
					size_t i_2 = _vn_vt_index[1][_right_value[i + 1]];
					relationship_table[i_1][i_2] = '=';
				}
				else if (i <= _right_value.size() - 2) {
					if (_vt_set.find(_right_value[i]) != _vt_set.end() && _vt_set.find(_right_value[i + 2]) != _vt_set.end() && _right_value[i + 1] <= 'Z' && _right_value[i + 1] >= 'A') {
						size_t i_1 = _vn_vt_index[1][_right_value[i]];
						size_t i_2 = _vn_vt_index[1][_right_value[i + 2]];
						relationship_table[i_1][i_2] = '=';
					}
				}
				// a, b 优先级a低于b
				if (_vt_set.find(_right_value[i]) != _vt_set.end() && _right_value[i + 1] <= 'Z' && _right_value[i + 1] >= 'A') {
					std::vector<bool> _first_vt_temp = _first_vt[_vn_vt_index[0][_right_value[i + 1]]];
					for (size_t j = 0; j < _first_vt_temp.size(); ++j) {
						if (_first_vt_temp[j]) {
							size_t i_1 = _vn_vt_index[1][_right_value[i]];
							size_t i_2 = _vn_vt_index[1][_vt_container[j]];
							relationship_table[i_1][i_2] = '<';
						}
					}
				}
				// a, b 优先级a高于b
				if (_vt_set.find(_right_value[i + 1]) != _vt_set.end() && _right_value[i] <= 'Z' && _right_value[i] >= 'A') {
					std::vector<bool> _last_vt_temp = _last_vt[_vn_vt_index[0][_right_value[i]]];
					for (size_t j = 0; j < _last_vt_temp.size(); ++j) {
						if (_last_vt_temp[j]) {
							size_t i_1 = _vn_vt_index[1][_vt_container[j]];
							size_t i_2 = _vn_vt_index[1][_right_value[i + 1]];
							relationship_table[i_1][i_2] = '>';
						}
					}
				}
			}
		}
	return relationship_table;
}

int main(int argc, char *argv[])
{
	// step1 解析文法
	std::string context_line;
	std::vector<std::vector<Context_Free*>> all_grammer;
	// win下用`ctrl+z`退出输入
	while (std::getline(std::cin, context_line)) {
		// 词法分析文法的输入，算是一个小型dsl的lexer
		all_grammer.push_back(tiny_lexical(context_line));
	}
	
	// step2 构造优先关系表
	std::vector<std::vector<bool>> flag;
	std::string vn_container;
	std::string vt_container = init_flag(flag, all_grammer);
	std::set<char> vt_set;
	for (auto x : vt_container)
		vt_set.insert(x);

	for (auto x : all_grammer) {
		for (auto y : x) {
			vn_container.push_back(y->get_left());
			break;
		}
	}

	std::vector<std::map<char, size_t>> vn_vt_index;	// 构造vn, vt 到index的映射
	std::map<char, size_t> vn_map, vt_map;
	for (size_t i = 0; i != vn_container.size(); ++i) {
		if (!vn_map[vn_container[i]])
			vn_map[vn_container[i]] = i;
	}
	for (size_t i = 0; i != vt_container.size(); ++i) {
		if (!vt_map[vt_container[i]])
			vt_map[vt_container[i]] = i;
	}
	vn_vt_index.push_back(vn_map);
	vn_vt_index.push_back(vt_map);
	std::vector<std::vector<bool>> last_vt = calc_last_vt(flag, all_grammer, vt_set, vn_vt_index);
	std::vector<std::vector<bool>> first_vt = calc_first_vt(flag, all_grammer, vt_set, vn_vt_index);

	std::vector<std::vector<char>> relationship_table = init_relationship_table(vt_container);
	relationship_table = calc_relationship_table(last_vt, first_vt, all_grammer, vt_set, vn_vt_index, relationship_table, vt_container);

	for (auto x : vt_container) {
		std::cout << x << " ";
	}
	std::cout << std::endl;
	for (auto x : relationship_table) {
		for (auto y : x) {
			std::cout << y << " ";
		}
		std::cout << std::endl;
	}
	return 0;
}