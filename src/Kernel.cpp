/*
 * Copyright (c) 2017 Jon Olsson <jlo@wintermute.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "kernel/Kernel.hpp"
#include <log/log.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tuple>
#include <thread>
#include <utility>
#include <atomic>

namespace {

template<typename T>
struct Vertex {
  T* value;
  std::vector<Vertex<T>*> adjList;
};

inline
static
auto
createGraph(
    std::vector<Vertex<Kernel::Module>> * moduleGraph,
    std::vector<std::tuple<Kernel::Module*, std::vector<std::string>>> const& modulesAndDependencies
    ) -> bool
{
  std::unordered_map<std::string, Kernel::Module*> moduleMap;
  std::unordered_map<Kernel::Module*, Vertex<Kernel::Module>*> vertexMap;
  moduleGraph->reserve(modulesAndDependencies.size());
  for (auto const & module_dependencies : modulesAndDependencies) {
    auto const module = std::get<0>(module_dependencies);
    moduleMap[module->name()] = module;
    Vertex<Kernel::Module> vertex;
    vertex.value = module;
    moduleGraph->push_back(vertex);
    vertexMap[module] = moduleGraph->data() + moduleGraph->size() - 1;
  }

  for (auto const & module_dependencies : modulesAndDependencies) {
    auto const module = std::get<0>(module_dependencies);
    auto const names = std::get<1>(module_dependencies);
    auto const vertex = vertexMap.find(module)->second;
    for (auto & name : names) {
      auto const itr = moduleMap.find(name);
      if (itr != moduleMap.end()) {
        auto const dependency = vertexMap.find(itr->second)->second;
        LOG(LOG_DEBUG, "Kernel: adding '%s' as dependency of '%s'", name.c_str(), module->name().c_str());
        dependency->adjList.push_back(vertex);
      } else {
        LOG(LOG_ERROR, "Kernel: dependency '%s' not found", name.c_str());
        return false;
      }
    }
  }
  return true;
}

inline
static
auto
sort(std::vector<Kernel::Module*> * sortedModules,
     std::vector<std::tuple<Kernel::Module*, std::vector<std::string>>> const& modulesAndDependencies
    ) -> bool
{

  std::vector<Vertex<Kernel::Module>> moduleGraph;
  if (!createGraph(&moduleGraph, modulesAndDependencies)) {
    return false;
  }

  std::vector<Kernel::Module*> sm;
  std::unordered_set<Vertex<Kernel::Module>*> vertices;
  for (auto & vertex : moduleGraph) {
    vertices.insert(&vertex);
  }
  for (auto & n : moduleGraph) {
    for (auto & m : n.adjList) {
      auto const itr = vertices.find(m);
      if (itr != vertices.end()) {
        vertices.erase(itr);
      }
    }
  }
  while (!vertices.empty()) {
    auto const itr = vertices.begin();
    auto const vertex = *itr;
    sm.push_back(vertex->value);
    vertices.erase(itr);
    auto const adjList = vertex->adjList;
    vertex->adjList.clear();
    for (auto & m : adjList) {
      for (auto & o : moduleGraph) {
        for (auto & p : o.adjList) {
          if (m == p) {
            goto _next;
          }
        }
      }
      vertices.insert(m);
_next:
      {
      }
    }
  }
  for (auto & m : moduleGraph) {
    if (!m.adjList.empty()) {
      LOG(LOG_ERROR, "Kernel: detected cyclic dependency between modules");
      return false;
    }
  }
  sortedModules->swap(sm);
  return true;
}

} // !namespace

struct KernelImpl {
  explicit KernelImpl()
    : status(Kernel::Status::IDLE)
    , modules()
    , modulesAndDependencies()
    , kernelThread(nullptr)
  {
  }
  ~KernelImpl()
  {
    status.store(Kernel::Status::IDLE);
    kernelThread->join();
  }
  auto start() -> void;
  std::atomic<Kernel::Status> status;
  Kernel * kernel;
  std::vector<std::unique_ptr<Kernel::Module>> modules;
  std::vector<std::tuple<Kernel::Module*, std::vector<std::string>>> modulesAndDependencies;
  std::unique_ptr<std::thread> kernelThread;
};

auto
KernelImpl::start() -> void
{
  LOG(LOG_DEBUG, "KernelImpl::start: starting");

  std::vector<Kernel::Module*> sortedModules;
  if (!sort(&sortedModules, modulesAndDependencies)) {
    status.store(Kernel::Status::IDLE);
    return;
  }
  LOG(LOG_DEBUG, "KernelImpl::start: scheduled order:");
  for (auto itr = sortedModules.begin(); itr != sortedModules.end(); ++itr) {
    LOG(LOG_DEBUG, "KernelImpl::start:  %s", (*itr)->name().c_str());
  }

  for (auto itr = sortedModules.begin(); itr != sortedModules.end(); ++itr) {
    (*itr)->init();
  }
  status.store(Kernel::Status::RUNNING);
  while (status.load() == Kernel::Status::RUNNING) {
    for (auto itr = sortedModules.begin(); itr != sortedModules.end(); ++itr) {
      (*itr)->tick();
    }
    std::this_thread::yield();
  }
  for (auto itr = sortedModules.rbegin(); itr != sortedModules.rend(); ++itr) {
    (*itr)->halt();
  }
  LOG(LOG_DEBUG, "KernelImpl::start: finished");
}

Kernel::Module::Module(std::string const & name)
  : _name(name)
  , _kernel(nullptr)
{
  LOG(LOG_DEBUG, "Kernel: Module '%s' created", _name.c_str());
}

Kernel::Module::~Module()
{
  LOG(LOG_DEBUG, "Kernel: Module '%s' destructed", _name.c_str());
}

Kernel::Kernel()
  : _p(std::make_unique<KernelImpl>())
{
  LOG(LOG_DEBUG, "Kernel: created");
}

Kernel::~Kernel()
{
  LOG(LOG_DEBUG, "Kernel: destructed");
}

auto
Kernel::add(std::unique_ptr<Module> module, std::vector<std::string> const& dependencies) -> bool
{
  if (_p->status.load() != Status::IDLE) {
    LOG(LOG_ERROR, "Kernel::add: Kernel is not idle");
    return false;
  }
  auto const name = module->name();
  if (find(name)) {
    LOG(LOG_WARNING, "Kernel::add: module '%s' already added", name.c_str());
    return false;
  }
  module->_kernel = this;
  _p->modulesAndDependencies.push_back(std::make_tuple(module.get(), dependencies));
  _p->modules.push_back(std::move(module));
  LOG(LOG_DEBUG, "Kernel::add: module '%s' added", name.c_str());
  return true;
}

auto
Kernel::remove(std::string const & name) -> std::unique_ptr<Module>
{
  if (_p->status.load() != Status::IDLE) {
    LOG(LOG_ERROR, "Kernel::add: Kernel is not idle");
    return nullptr;
  }
  for (auto i = 0u; i < _p->modules.size(); ++i) {
    if (_p->modules[i]->name() == name) {
      if (std::get<0>(_p->modulesAndDependencies[i])->name() == name) {
        _p->modulesAndDependencies[i].swap(_p->modulesAndDependencies.back());
        _p->modulesAndDependencies.pop_back();
      } else {
        LOG(LOG_ERROR, "Kernel::remove: internal error: inconsistent module arrays");
      }
      _p->modules[i].swap(_p->modules.back());
      auto module = std::move(_p->modules.back());
      _p->modules.pop_back();
      LOG(LOG_DEBUG, "Kernel::remove: module '%s' removed", name.c_str());
      return module;
    }
  }
  LOG(LOG_WARNING, "Kernel::remove: Module '%s' not found", name.c_str());
  return nullptr;
}

auto
Kernel::find(std::string const & name) const -> Kernel::Module *
{
  for (auto & module : _p->modules) {
    if (module->name() == name) {
      return module.get();
    }
  }
  return nullptr;
}

auto
Kernel::start() -> bool
{
  LOG(LOG_DEBUG, "Kernel::start: starting");

  _p->status.store(Status::STARTING);
  _p->kernelThread.reset(new std::thread(&KernelImpl::start, _p.get()));

  while (_p->status.load() == Status::STARTING) {
    std::this_thread::yield();
  }
  return _p->status.load() == Status::RUNNING;
}

auto
Kernel::stop() -> void
{
  _p->status.store(Status::IDLE);
  LOG(LOG_DEBUG, "Kernel::stop: stopped");
}

auto
Kernel::status() const -> Status
{
  return _p->status.load();
}
