#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <queue>
#include <algorithm>

namespace suffix_index {

typedef typename std::iterator_traits<typename std::vector<char>::iterator>::difference_type string_index;

class node;

class substring {
protected:
	long index; // the index of the parent string
	string_index left; // the left bound of the string
	string_index right; // the right bound of the string
	string_index * end; // the pointer of the end when the right side is still changing
	bool changing_right;
public:
	substring(long index, string_index left, string_index * end) :
			index(index), left(left), end(end), right(0), changing_right(true) {
	}

	substring(long index, string_index left, string_index right):index(index), left(left), right(right),changing_right(false), end(nullptr){

	}

	substring():index(0), left(0), right(0), end(nullptr),changing_right(true){

	}

	long getIndex() const {
		return index;
	}

	void setIndex(long index) {
		this->index = index;
	}

	char getChar(const std::vector<std::vector<char> > indices, long offset) const {
		return indices[getIndex()][getLeft()+offset];
	}

	bool isOffsetValid(long offset) const{
		return getLeft() + offset <= getRight();
	}

	string_index getLeft() const {
		return left;
	}

	void setLeft(string_index left) {
		this->left = left;
	}

	string_index getRight() const {
		if (changing_right){
			return *end;
		}else{
			return right;
		}
	}

	void setRight(string_index right) {
		this->right = right;
	}

	void stop_changing(){
		this->changing_right = false;
		this->right = *this->end;
	}

	bool is_changing() const {
		return this->changing_right;
	}

	string_index* getEnd() const {
		return end;
	}
};

struct substring_cmp {
    bool operator()(const substring& lhs, const substring& rhs) const {
        return lhs.getLeft() < rhs.getLeft();
    }
};

class link {
private:
	//std::set<substring, substring_cmp> labels; //if there is no threshold on approximation, we could keep only one label, which is more efficient when there is split
	substring label;
	node * target;
public:
	link(substring substr, node * target): target(target), label(substr){
	}
	link(): target(nullptr){

	}

	substring & getLabel() {
		return label;
	}

	void setLabel(const substring &label) {
		this->label = label;
	}

	node* getTarget() {
		return target;
	}

	void setTarget(node *target) {
		this->target = target;
	}
};

class node {
private:
	std::unordered_map<char, link> children;
	std::unordered_set<int> attribtues;
	node * suffix_link;
public:
	~node(){
		// to avoid recursive deletion
	}

	node():suffix_link(nullptr){

	}

	const std::unordered_set<int>& getAttribtues() const {
		return attribtues;
	}

	void setAttribtues(const std::unordered_set<int> attribtues) {
		this->attribtues = attribtues;
	}

	void addAttribtue(const int & attribute) {
		this->attribtues.insert(attribute);
	}

	std::unordered_map<char, link>& getChildren() {
		return children;
	}

	bool isLeaf() const {
		return children.empty();
	}

	link * getLink(char c) {
		if (children.find(c)!= children.end()){
			return & children[c];
		}else{
			return nullptr;
		}
	}

	void setChildren(const std::unordered_map<char, link> &children) {
		this->children = children;
	}

	void addChild(char ch, const link & l){
		this->children[ch] = l;
	}

	node* getSuffixLink() {
		return suffix_link;
	}

	void setSuffixLink(node * suffixLink) {
		suffix_link = suffixLink;
	}
};

class leaf: public node{
};

class parameters{
public:
	string_index remaining;
	node * active_node;
	string_index active_edge;
	string_index active_length;
	string_index end;
	long str_index;

	parameters(long str_index): remaining(0), active_node(nullptr), active_edge(-1), active_length(0), end(-1), str_index(str_index){

	}

	string_index getActiveEdge() const {
		return active_edge;
	}

	void setActiveEdge(string_index activeEdge) {
		active_edge = activeEdge;
	}

	string_index getActiveLength() const {
		return active_length;
	}

	void setActiveLength(string_index activeLength) {
		active_length = activeLength;
	}

	const node* getActiveNode() const {
		return active_node;
	}

	void setActiveNode(node * activeNode) {
		active_node = activeNode;
	}

	string_index getEnd() const {
		return end;
	}

	void setEnd(string_index end) {
		this->end = end;
	}

	string_index getRemaining() const {
		return remaining;
	}

	void setRemaining(string_index remaining) {
		this->remaining = remaining;
	}
};

class end_of_path_exception: public std::exception{
};
class index_build_exception: public std::exception{
};

class suffix_tree {
private:
	std::vector<std::vector<char> > indices;
	std::vector<parameters*> param_indices;
	int approximate;
	parameters* param;
	node root;

	void index_string(const std::vector<char> & str, int value){
		this->indices.push_back(str);
		long str_index = this->indices.size() - 1;
		param = new parameters(str_index);
		this->param_indices.push_back(param);
		param->setActiveNode(&root);
		for (long i=0; i < str.size(); ++i){
			index_step(i, value);
		}
		/*
		if (param->remaining != 0){
			throw index_build_exception();
		}
		*/
	}

	node * select_node(long i){
		std::unordered_map<char, link> & children =
		param->active_node->getChildren();
		std::unordered_map<char, link>::iterator it = children.find(indices[param->str_index][i]);
		if (it==children.end())
			return nullptr;
		else
			return it->second.getTarget();
	}


	node * select_node(){
		std::unordered_map<char, link> & children =
		param->active_node->getChildren();
		std::unordered_map<char, link>::iterator it = children.find(indices[param->str_index][param->active_edge]);
		return it->second.getTarget();
	}

	link & select_link(){
		std::unordered_map<char, link> & children =
		param->active_node->getChildren();
		std::unordered_map<char, link>::iterator it = children.find(indices[param->str_index][param->active_edge]);
		return it->second;
	}

	char next_char(long i){
		std::unordered_map<char, link> & children =
		param->active_node->getChildren();
		std::unordered_map<char, link>::iterator it = children.find(indices[param->str_index][param->active_edge]);
		const substring & label = it->second.getLabel();
		const std::vector<char> & str = indices[label.getIndex()];
		if((label.getRight()-label.getLeft()) >= param->active_length){
			return str[label.getLeft() + param->active_length];
		}
		if((label.getRight()-label.getLeft()) + 1== param->active_length){
			if(it->second.getTarget()->getLink(indices[param->str_index][i]) != nullptr){
				return indices[param->str_index][i];
			}
		}else{
			param->active_node = it->second.getTarget();
			param->active_length = param->active_length - (label.getRight()-label.getLeft()) -1;
			param->active_edge = param->active_edge + (label.getRight()-label.getLeft()) + 1;
			return next_char(i);
		}
		throw end_of_path_exception();
	}

	void index_step(long i, int value){
		node * last_created_internal_node = nullptr;
		++param->end;
		++param->remaining;
		while (param->remaining > 0){
			if(param->active_length == 0){
				if(select_node(i) != nullptr){
					param->active_edge = i;
					++param->active_length;

					node * n = select_node(i);
					if (n->isLeaf())
						n->addAttribtue(value);

					break;
				}else{
					node * l = new node();
					l->addAttribtue(value);
					param->active_node->addChild(indices[param->str_index][i], link(
							substring(param->str_index, i, &param->end),
							l
							));
					--param->remaining;
				}
			} else {
				try{
					char ch = next_char(i);
					if(ch==indices[param->str_index][i]){
						if(last_created_internal_node != nullptr){
							last_created_internal_node->setSuffixLink(select_node());
						}

						/*update node value when it is used to find longest common substring*/
						/*otherwise it would more efficient to keep only walk_down and break*/
						/*lcs becomes to find a path on which every values are contained on every nodes*/
						link & l = select_link();
						const substring & label = l.getLabel();
						if (param->active_length == (label.getRight()-label.getLeft())){
							l.getTarget()->addAttribtue(value);
							if(param->active_node != &root){
								param->active_node = param->active_node->getSuffixLink();
							}else{
								param->active_edge = param->active_edge + 1;
								--param->active_length;
							}
							--param->remaining;
						}else{
							walk_down(i);
							break;
						}
					}else{
						link & l = select_link();
						// add a new node to split
						node * old_node = l.getTarget();
						node * new_node = new node();
						new_node->setAttribtues(old_node->getAttribtues());
						node * new_leaf = new node();
						new_leaf->addAttribtue(value);
						new_node->addAttribtue(value);
						link l_old = link();
						link l_leaf = link();
						l_old.setTarget(old_node);
						substring & label = l.getLabel();
						if(label.is_changing()){
							l_old.setLabel(substring(
									label.getIndex(),
									label.getLeft() + param->active_length, label.getEnd()
							));
							label.stop_changing();
							label.setRight(label.getLeft() + param->active_length-1);
						} else {
							long old_right = label.getRight();
							label.setRight(label.getLeft()+param->active_length-1);
							l_old.setLabel(substring(
									label.getIndex(),
									label.getLeft() + param->active_length,
									old_right
							));
						}
						l.setTarget(new_node);
						l_old.setTarget(old_node);
						const substring & first_label = l_old.getLabel();
						new_node->addChild(indices[first_label.getIndex()][first_label.getLeft()], l_old);

						l_leaf.setLabel(substring(param->str_index,
								i, &param->end));
						l_leaf.setTarget(new_leaf);
						new_node->addChild(indices[param->str_index][i], l_leaf);

						if(last_created_internal_node != nullptr){
							last_created_internal_node->setSuffixLink(new_node);
						}

						last_created_internal_node = new_node;
						new_node->setSuffixLink(&root);
						if(param->active_node != &root){
							param->active_node = param->active_node->getSuffixLink();
						}else{
							param->active_edge = param->active_edge + 1;
							--param->active_length;
						}
						--param->remaining;
					}
				} catch (const end_of_path_exception &e) {
					node * n = select_node();
					node * new_node = new node();
					new_node->addAttribtue(value);
					n->addChild(indices[param->str_index][i],
							link(substring(param->str_index,
									i,&param->end),
									new_node));
					if (last_created_internal_node != nullptr){
						last_created_internal_node->setSuffixLink(n);
					}
					last_created_internal_node = n;
					if (param->active_node != &root){
						param->active_node = param->active_node->getSuffixLink();
					}else{
						++param->active_edge;
						--param->active_length;
					}
					--param->remaining;
				}
			}
		}
	}

	void walk_down(long i){
		link & l = select_link();
		const substring & label = l.getLabel();
		if(param->active_length > (label.getRight()-label.getLeft())){
			param->active_node = l.getTarget();
            param->active_length = param->active_length - (label.getRight()-label.getLeft());
			param->active_edge = i;
		}else{
			++param->active_length;
		}
	}

public:
	static const char unique_char = '$';

	~suffix_tree() {
		// to avoid recursive deletion
        std::queue<node*> del_q;
        del_q.push(&root);
        while (!del_q.empty()) {
            node *current = del_q.front();del_q.pop();
            for (auto it : current->getChildren()) {
                del_q.push(it.second.getTarget());
            }
            if (&root != current) {
                delete current;
            }
        }
        for (parameters * p: param_indices){
        	delete p;
        }
	}

	suffix_tree(int approximate) :
			approximate(approximate), param(nullptr){
	}
	suffix_tree() :
			suffix_tree(-1) {

	}
	void add_string(const std::string &word, int value) {
		std::vector<char> char_list(word.begin(), word.end());
		add_string(char_list, value);
	}

	void add_string(const std::vector<char> &word, int value) {
		this->index_string(word, value);
	}

	std::unordered_set<int> search_string(const std::string word) {
		std::vector<char> char_list(word.begin(), word.end());
		return search_string(char_list);

	}

	std::unordered_set<int> search_string(const std::vector<char> text) {
		std::unordered_set<int> ans;
		node * current = &root;
		for (size_t i=0; i < text.size(); ++i){
			if (current != &this->root){
				if(approximate<0){
					ans.insert(current->getAttribtues().begin(),
							current->getAttribtues().end());
				}
			}
			link * l = current->getLink(text[i]);
			if (l==nullptr){
				break;
			}else{
				const substring & link_label = l->getLabel();
				long offset = 0;
				while(link_label.isOffsetValid(offset) && i < text.size() &&link_label.getChar(this->indices, offset) == text[i]){
					++offset;
					++i;
				}
				node * target = l->getTarget();
				if (i >= text.size()){
					if(approximate<0){
						ans.insert(target->getAttribtues().begin(),
								target->getAttribtues().end());
					}else{
						if(target->isLeaf()){
							if ((link_label.getRight()-link_label.getLeft()-offset+1) <= approximate){
								ans.insert(target->getAttribtues().begin(),
										target->getAttribtues().end());
							}
						}
					}
				}else{
					--i;
					current = target;
				}
			}
		}
		return ans;
	}

	bool is_suffix(const std::string word) {
		std::vector<char> char_list(word.begin(), word.end());
		return is_suffix(char_list);

	}

	bool is_suffix(const std::vector<char> text) {
		node * current = &root;
		for (size_t i=0; i < text.size(); ++i){
			link * l = current->getLink(text[i]);
			if (l==nullptr){
				break;
			}else{
				const substring & link_label = l->getLabel();
				long offset = 0;
				while(link_label.isOffsetValid(offset) && i < text.size() &&link_label.getChar(this->indices, offset) == text[i]){
					++offset;
					++i;
				}
				node * target = l->getTarget();
				if (i >= text.size()){
					if(target->isLeaf()){
						if ((link_label.getRight()-link_label.getLeft()-offset+1) == 0){
							return true;
						}
					}
				}else{
					--i;
					current = target;
				}
			}
		}
		return false;
	}

	bool is_substring(const std::string word) {
		std::vector<char> char_list(word.begin(), word.end());
		return is_substring(char_list);

	}

	bool is_substring(const std::vector<char> text){
		node * current = &root;
		for (size_t i=0; i < text.size(); ++i){
			link * l = current->getLink(text[i]);
			if (l==nullptr){
				break;
			}else{
				const substring & link_label = l->getLabel();
				long offset = 0;
				while(link_label.isOffsetValid(offset) && i < text.size() &&link_label.getChar(this->indices, offset) == text[i]){
					++offset;
					++i;
				}
				node * target = l->getTarget();
				if (i >= text.size()){
					return true;
				}else{
					--i;
					current = target;
				}
			}
		}
		return false;
	}
};

const char suffix_tree::unique_char;

}

int main(int argc, char **argv) {
	std::cout << "Hello World" << std::endl;
	suffix_index::suffix_tree st(0);
	st.add_string("Hello", 1);
	st.add_string("Hurry", 2);
	st.add_string("mississippi", 4);
	st.add_string("Huhao", 3);
	st.add_string("Puhao", 5);
	st.add_string("Hfhao", 6);
	st.add_string("Puhax", 7);
	for (int v: st.search_string("o")){
		std::cout << v << std::endl;
	}
	std::cout << std::endl;
	for (int v: st.search_string("hao")){
		std::cout << v << std::endl;
	}
	std::cout << std::endl;
	for (int v: st.search_string("Puhax")){
		std::cout << v << std::endl;
	}
	for (int v: st.search_string("Hurry")){
		std::cout << v << std::endl;
	}
	for (int v: st.search_string("mississippi")){
		std::cout << v << std::endl;
	}

	std::cout << st.is_suffix("o") << std::endl;
	std::cout << st.is_suffix("fhao") << std::endl;
	std::cout << st.is_suffix("fha") << std::endl;
	std::cout << st.is_suffix("Puhax") << std::endl;
	std::cout << st.is_suffix("Hurry") << std::endl;
	std::cout << st.is_suffix("mississippi") << std::endl;
	std::cout << st.is_substring("pma") << std::endl;
	std::cout << st.is_substring("fha") << std::endl;
	return 0;
}
