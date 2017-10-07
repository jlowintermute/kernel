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

#ifndef kernel_Kernel_hpp
#define kernel_Kernel_hpp

#include <string>
#include <vector>
#include <memory>

class KernelImpl;

class Kernel {
public:

  enum class Status { IDLE, STARTING, RUNNING };

  class Module {
  public:
    explicit Module(std::string const & name);
    virtual ~Module();
    virtual auto init() -> void {}
    virtual auto tick() -> void {}
    virtual auto halt() -> void {}
    auto name() const -> std::string { return _name; }
  protected:
    auto kernel() const -> Kernel * { return _kernel; }
  private:
    friend Kernel;
    std::string _name;
    Kernel * _kernel;
  };

  Kernel();
  ~Kernel();

  using DependencyList = std::vector<std::string>;

  auto add(std::unique_ptr<Module> module, DependencyList const& dependencies = DependencyList()) -> bool;
  auto remove(std::string const & name) -> std::unique_ptr<Module>;
  auto find(std::string const & name) const -> Module *;

  auto start() -> bool;
  auto stop() -> void;

  auto status() const -> Status;

private:
  std::unique_ptr<KernelImpl> _p;
};

#endif  // !kernel_Kernel_hpp
