#include<iostream>
#include<unordered_map>
#include<functional>
#include<memory>
#include<sstream>
#include <assert.h>
using namespace std;

template<typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K> >
class EvictingMap {
 private:
  class LinkNode;
  struct MapNode {
    MapNode() :value(nullptr),link_node(nullptr) {

    }
    std::shared_ptr<V> value;
    LinkNode* link_node;
  };

  struct LinkNode {
    LinkNode():pre(nullptr),nxt(nullptr) {}
    LinkNode*pre;
    LinkNode*nxt;
    typename std::unordered_map<K, MapNode>::iterator it;
    std::string DebugString() {
      stringstream os;
      os<<"(k : "<<it->first<<", v : "<<*it->second.value<<", pre : "<<pre<<", next : "<<nxt<<")";
      return os.str();
    }
  };

  struct LinkManager {
    LinkManager(): head(nullptr),len(0) {
    }
    void Push(LinkNode* node) {
      len++;
      if(head == nullptr) {
//                cout<<"first init"<<endl;
        node->pre = node->nxt = node;
        head = node;
        return ;
      }
      // 因为是循环链表
      LinkNode* tail = head->pre;

      node->pre = tail;
      node->nxt = tail->nxt;
      tail->nxt = node;
      head->pre = node;

    }
    void Erase(LinkNode* node) {
      len --;
      if(node->nxt == node && node->pre == node) {
        head = nullptr;
        return ;
      }
      if (node == head) {
        head = head->nxt;
      }
      LinkNode* pre = node->pre;
      LinkNode* nxt = node->nxt;
      pre->nxt = nxt;
      nxt->pre = pre;
    }
    LinkNode* Pop() {
      if (head == nullptr) {
        return nullptr;
      }
      LinkNode* top = head;
      Erase(top);
      return top;
    }
    std::string DebugString() {
      ostringstream os;
      LinkNode* st = head;
      if(st) {
        do {
          os << st->DebugString() << "; ";
          st = st->nxt;
        } while(st!=head);

      }
      return os.str();

    }
    uint32_t Size() {
      return len;
    }
    LinkNode* head;
    uint32_t len;
  };
 public:

  EvictingMap(uint32_t max_size, uint32_t once_del_size = 1) : max_size(max_size), clear_size(once_del_size) {
  }
  ~EvictingMap() {
    while(_list.Size() > 0) {
      LinkNode* node = _list.Pop();
      //cout<<"~()"<<node->DebugString()<<endl;
      delete node;
    }
  }

  void SetEvictingCallback(const std::function<void(const K&, const V&)>& call) {
    call_back = call;
  }

  uint32_t Size() {
    return _map.size();
  }

  uint32_t ListSize() {
    return _list.Size();
  }

  V& operator [] (const K& key) {
    auto& map_val = _map[key];
    if (map_val.link_node == nullptr) {
      LinkNode* node(new LinkNode());
      node->it = _map.find(key);
      _list.Push(node);
      map_val.link_node = node;
    } else {
      LinkNode* node = map_val.link_node;
      _list.Erase(node);
      _list.Push(node);
    }
    if (map_val.value == nullptr) {
      map_val.value.reset(new V());
    }
    CheckEvict();
    return *map_val.value;
  }

  bool Put(const K& key, const V& value) {
    auto& map_val = _map[key];
    if (map_val.link_node == nullptr) {
      LinkNode* node(new LinkNode());
      node->it = _map.find(key);
      _list.Push(node);
      map_val.link_node = node;
    } else {
      LinkNode* node = map_val.link_node;
      _list.Erase(node);
      _list.Push(node);
    }
    if (map_val.value == nullptr) {
      map_val.value.reset(new V(value));
    } else {
      *map_val.value = value;
    }
    CheckEvict();
    return true;
  }
  V* Get(const K& key) {
    auto it = _map.find(key);
    if (it == _map.end()) {
      //没找到的情况
      return nullptr;
    }
    LinkNode* node = it->second.link_node;
    _list.Erase(node);
    _list.Push(node);
    return it->second.value.get();
  }
  bool Erase(const K& key) {
    auto it = _map.find(key);
    if (it == _map.end()) {
      return false;
    }
    auto node = it->second.link_node;
    _list.Erase(node);
    delete node;
    return true;
  }
  std::string ListDebugString() {
    return _list.DebugString();
  }
 private:
  void CheckEvict() {
    if(_map.size() > max_size) {
      uint32_t del_size = min(clear_size, _map.size());
//            cout<<"del "<<del_size<<endl;
      for(int i = 0; i < del_size; ++i) {
        LinkNode* node = _list.Pop();
        assert(node);
        typename std::unordered_map<K, MapNode>::iterator& it = node->it;
        if(call_back) {
          call_back(it->first, *(it->second.value));
        }
        _map.erase(node->it);
        delete node;
      }
    }
  }
  std::unordered_map<K, MapNode, Hash, KeyEqual> _map;
  LinkManager _list;
  uint32_t max_size;
  //每次clear的size
  uint32_t clear_size;
  std::function<void(const K&, const V&)> call_back;
};
