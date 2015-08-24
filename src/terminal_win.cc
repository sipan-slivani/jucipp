#include "terminal.h"
#include <iostream>
#include "logging.h"
#include "singletons.h"
#include <unistd.h>

#include <iostream> //TODO: remove
using namespace std; //TODO: remove

Terminal::InProgress::InProgress(const std::string& start_msg): stop(false) {
  waiting_print.connect([this](){
    Singleton::terminal()->async_print(line_nr-1, ".");
  });
  start(start_msg);
}

Terminal::InProgress::~InProgress() {
  stop=true;
  if(wait_thread.joinable())
    wait_thread.join();
}

void Terminal::InProgress::start(const std::string& msg) {
  line_nr=Singleton::terminal()->print(msg+"...\n");
  wait_thread=std::thread([this](){
    size_t c=0;
    while(!stop) {
      if(c%100==0)
        waiting_print();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      c++;
    }
  });
}

void Terminal::InProgress::done(const std::string& msg) {
  if(!stop) {
    stop=true;
    Singleton::terminal()->async_print(line_nr-1, msg);
  }
}

void Terminal::InProgress::cancel(const std::string& msg) {
  if(!stop) {
    stop=true;
    Singleton::terminal()->async_print(line_nr-1, msg);
  }
}

Terminal::Terminal() {
  bold_tag=get_buffer()->create_tag();
  bold_tag->property_weight()=PANGO_WEIGHT_BOLD;
  
  signal_size_allocate().connect([this](Gtk::Allocation& allocation){
    auto iter=get_buffer()->end();
    if(iter.backward_char()) {
      auto mark=get_buffer()->create_mark(iter);
      scroll_to(mark, 0.0, 1.0, 1.0);
      get_buffer()->delete_mark(mark);
    }
  });
  
  async_print_dispatcher.connect([this](){
    async_print_strings_mutex.lock();
    if(async_print_strings.size()>0) {
      for(auto &string_bold: async_print_strings)
        print(string_bold.first, string_bold.second);
      async_print_strings.clear();
    }
    async_print_strings_mutex.unlock();
  });
  async_print_on_line_dispatcher.connect([this](){
    async_print_on_line_strings_mutex.lock();
    if(async_print_on_line_strings.size()>0) {
      for(auto &line_string: async_print_on_line_strings)
        print(line_string.first, line_string.second);
      async_print_on_line_strings.clear();
    }
    async_print_on_line_strings_mutex.unlock();
  });
}

int Terminal::execute(const std::string &command, const boost::filesystem::path &path) {
  /*std::string cd_path_and_command;
  if(path!="") {
    //TODO: Windows...
    cd_path_and_command="cd \""+path.string()+"\" && "+command;
  }
  else
    cd_path_and_command=command;
    
  int stdin_fd, stdout_fd, stderr_fd;
  auto pid=popen3(cd_path_and_command.c_str(), stdin_fd, stdout_fd, stderr_fd);
  
  if (pid<=0) {
    async_print("Error: Failed to run command: " + command + "\n");
    return -1;
  }
  else {
    std::thread stderr_thread([this, stderr_fd](){
      char buffer[1024];
      ssize_t n;
      while ((n=read(stderr_fd, buffer, 1024)) > 0) {
        std::string message;
        for(ssize_t c=0;c<n;c++)
          message+=buffer[c];
        async_print(message, true);
      }
    });
    stderr_thread.detach();
    std::thread stdout_thread([this, stdout_fd](){
      char buffer[1024];
      ssize_t n;
      INFO("read");
      while ((n=read(stdout_fd, buffer, 1024)) > 0) {
        std::string message;
        for(ssize_t c=0;c<n;c++)
          message+=buffer[c];
        async_print(message);
      }
    });
    stdout_thread.detach();
    
    int exit_code;
    waitpid(pid, &exit_code, 0);
    close(stdin_fd);
    close(stdout_fd);
    close(stderr_fd);
    
    return exit_code;
  }*/
  return -1;
}

void Terminal::async_execute(const std::string &command, const boost::filesystem::path &path, std::function<void(int exit_code)> callback) {
  /*std::thread async_execute_thread([this, command, path, callback](){
    std::string cd_path_and_command;
    if(path!="") {
    
      //TODO: Windows...
      cd_path_and_command="cd \""+path.string()+"\" && "+command;
    }
    else
      cd_path_and_command=command;
      
    int stdin_fd, stdout_fd, stderr_fd;
    async_executes_mutex.lock();
    stdin_buffer.clear();
    auto pid=popen3(cd_path_and_command.c_str(), stdin_fd, stdout_fd, stderr_fd);
    async_executes.emplace_back(pid, stdin_fd);
    async_executes_mutex.unlock();
    
    if (pid<=0) {
      async_print("Error: Failed to run command: " + command + "\n");
      if(callback)
        callback(-1);
    }
    else {
      std::thread stderr_thread([this, stderr_fd](){
        char buffer[1024];
        ssize_t n;
        while ((n=read(stderr_fd, buffer, 1024)) > 0) {
          std::string message;
          for(ssize_t c=0;c<n;c++)
            message+=buffer[c];
          async_print(message, true);
	}
      });
      stderr_thread.detach();
      std::thread stdout_thread([this, stdout_fd](){
        char buffer[1024];
        ssize_t n;
        INFO("read");
        while ((n=read(stdout_fd, buffer, 1024)) > 0) {
          std::string message;
          for(ssize_t c=0;c<n;c++)
            message+=buffer[c];
          async_print(message);
	}
      });
      stdout_thread.detach();
      
      int exit_code;
      waitpid(pid, &exit_code, 0);
      async_executes_mutex.lock();
      for(auto it=async_executes.begin();it!=async_executes.end();it++) {
        if(it->first==pid) {
          async_executes.erase(it);
          break;
        }
      }
      stdin_buffer.clear();
      close(stdin_fd);
      close(stdout_fd);
      close(stderr_fd);
      async_executes_mutex.unlock();
      
      if(callback)
        callback(exit_code);
    }
  });
  async_execute_thread.detach();*/
  if(callback)
    callback(-1);
}

void Terminal::kill_last_async_execute(bool force) {
  async_executes_mutex.lock();
  /*if(async_executes.size()>0) {
    if(force)
      kill(-async_executes.back().first, SIGTERM);
    else
      kill(-async_executes.back().first, SIGINT);
  }*/
  async_executes_mutex.unlock();
}

void Terminal::kill_async_executes(bool force) {
  async_executes_mutex.lock();
  /*for(auto &async_execute: async_executes) {
    if(force)
      kill(-async_execute.first, SIGTERM);
    else
      kill(-async_execute.first, SIGINT);
  }*/
  async_executes_mutex.unlock();
}

int Terminal::print(const std::string &message, bool bold){
  INFO("Terminal: PrintMessage");
  if(bold)
    get_buffer()->insert_with_tag(get_buffer()->end(), message, bold_tag);
  else
    get_buffer()->insert(get_buffer()->end(), message);
    
  auto iter=get_buffer()->end();
  if(iter.backward_char()) {
    auto mark=get_buffer()->create_mark(iter);
    scroll_to(mark, 0.0, 1.0, 1.0);
    get_buffer()->delete_mark(mark);
  }
  
  return get_buffer()->end().get_line();
}

void Terminal::print(int line_nr, const std::string &message){
  INFO("Terminal: PrintMessage at line " << line_nr);
  auto iter=get_buffer()->get_iter_at_line(line_nr);
  while(!iter.ends_line())
    iter++;
  get_buffer()->insert(iter, message);
}

std::shared_ptr<Terminal::InProgress> Terminal::print_in_progress(std::string start_msg) {
  std::shared_ptr<Terminal::InProgress> in_progress=std::shared_ptr<Terminal::InProgress>(new Terminal::InProgress(start_msg));
  return in_progress;
}

void Terminal::async_print(const std::string &message, bool bold) {
  async_print_strings_mutex.lock();
  bool dispatch=true;
  if(async_print_strings.size()>0)
    dispatch=false;
  async_print_strings.emplace_back(message, bold);
  async_print_strings_mutex.unlock();
  if(dispatch)
    async_print_dispatcher();
}

void Terminal::async_print(int line_nr, const std::string &message) {
  async_print_on_line_strings_mutex.lock();
  bool dispatch=true;
  if(async_print_on_line_strings.size()>0)
    dispatch=false;
  async_print_on_line_strings.emplace_back(line_nr, message);
  async_print_on_line_strings_mutex.unlock();
  if(dispatch)
    async_print_on_line_dispatcher();
}

bool Terminal::on_key_press_event(GdkEventKey *event) {
  async_executes_mutex.lock();
  if(async_executes.size()>0) {
    get_buffer()->place_cursor(get_buffer()->end());
    auto unicode=gdk_keyval_to_unicode(event->keyval);
    char chr=(char)unicode;
    if(unicode>=32 && unicode<=126) {
      stdin_buffer+=chr;
      get_buffer()->insert_at_cursor(stdin_buffer.substr(stdin_buffer.size()-1));
      scroll_to(get_buffer()->get_insert());
    }
    else if(event->keyval==GDK_KEY_BackSpace) {
      if(stdin_buffer.size()>0 && get_buffer()->get_char_count()>0) {
        auto iter=get_buffer()->end();
        iter--;
        stdin_buffer.pop_back();
        get_buffer()->erase(iter, get_buffer()->end());
        scroll_to(get_buffer()->get_insert());
      }
    }
    else if(event->keyval==GDK_KEY_Return) {
      stdin_buffer+='\n';
      write(async_executes.back().second, stdin_buffer.c_str(), stdin_buffer.size());
      get_buffer()->insert_at_cursor(stdin_buffer.substr(stdin_buffer.size()-1));
      scroll_to(get_buffer()->get_insert());
      stdin_buffer.clear();
    }
  }
  async_executes_mutex.unlock();
  return true;
}