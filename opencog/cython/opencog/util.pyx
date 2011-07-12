from libcpp cimport bool
from cython.operator cimport dereference as deref, preincrement as inc

from atomspace cimport string

cdef extern from "opencog/util/Logger.h" namespace "opencog":
    # Need to get around cython's lack of support for nested types
    enum loglevel "opencog::Logger::Level":
        NONE "opencog::Logger::NONE"
        ERROR "opencog::Logger::ERROR"
        WARN "opencog::Logger::WARN"
        INFO "opencog::Logger::INFO"
        DEBUG "opencog::Logger::DEBUG"
        FINE "opencog::Logger::FINE"
        BAD_LEVEL "opencog::Logger::BAD_LEVEL"
    cdef cppclass cLogger "opencog::Logger":
        void setLevel(loglevel lvl)
        loglevel getLevel()
        void setPrintToStdoutFlag(bool flag)

        void log(loglevel lvl, string txt)

        bool isEnabled(loglevel lvl)

    cdef loglevel string_to_log_level "opencog::Logger::getLevelFromString"(string s)
    cdef string log_level_to_string "opencog::Logger::getLevelString"(loglevel lvl)
    cLogger& logger()

cdef class Logger:
    cdef cLogger *clog
    def __init__(self):
        self.clog = &logger()
    def log(self, int lvl, txt):
        # why do we use an int for lvl? it is due to a weird cython issue that tries
        # to convert lvl into a long if the type of lvl is loglevel.
        # convert to string
        py_byte_string = txt.encode('UTF-8')
        # create temporary cpp string
        cdef string *msg = new string(py_byte_string)
        self.clog.log(<loglevel>lvl,deref(msg))
        # delete temporary string
        del msg
    def error(self, txt):
        self.log(ERROR,txt)
    def warn(self, txt):
        self.log(WARN,txt)
    def info(self, txt):
        self.log(INFO,txt)
    def debug(self, txt):
        self.log(DEBUG,txt)
    def fine(self, txt):
        self.log(FINE,txt)
    def is_enabled(self, int lvl):
        return self.clog.isEnabled(<loglevel>lvl)
    def use_stdout(self,use_it=True):
        self.clog.setPrintToStdoutFlag(use_it)
    def get_level(self):
        return self.clog.getLevel()
    def set_level(self,int lvl):
        self.clog.setLevel(<loglevel>lvl)
    def level_as_string(self):
        cdef string level_name
        level_name = log_level_to_string(<loglevel>self.clog.getLevel())
        return level_name.c_str()[:level_name.size()].decode('UTF-8')
    def set_level_from_string(self,level_name):
        level_name = level_name.upper()
        py_byte_string = level_name.encode('UTF-8')
        # create temporary cpp string
        cdef string *c_level_name = new string(py_byte_string)
        loglvl = string_to_log_level(deref(c_level_name))
        del c_level_name
        if (loglvl == BAD_LEVEL): raise ValueError("Bad level name")
        self.set_level(loglvl)
        return loglvl

log = Logger()
        





