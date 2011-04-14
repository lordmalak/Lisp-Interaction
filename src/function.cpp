
#include "function.hpp"

#include "logging.hpp"


namespace lisp {
    function::function(const arg_sym_list_t& arg_symbols,
                       cons_cell_ptr_t body)
        : object(),
          m_arg_symbols(arg_symbols),
          m_body(body)
    {
    }

    object_ptr_t function::operator()(environment* env, const cons_cell_ptr_t args)
    {
        using logging::log;
        using logging::DEBUG;

        // Create isolated environment.
        environment func_env(env);

        arg_sym_list_t::const_iterator arg_sym_iter = m_arg_symbols.begin();

        object_ptr_t cdr_ = nil();

        if(args)
            cdr_ = args;

        // Assign all given args to corresponding symbols
        // in the function environment.
        while(cdr_ != nil()) {
            object_ptr_t car_;

            if(cdr_->is_cons_cell())
                // Seems to be risky but it's not :-)
                car_ = boost::dynamic_pointer_cast<cons_cell>(cdr_)->car();
            else
                signal(env->get_symbol("wrong-type-argument"),
                       "listp " + cdr_->str());

            if(arg_sym_iter == m_arg_symbols.end())
                signal(env->get_symbol("wrong-number-of-arguments"),
                       args->car()->str());

            // Create new symbol in function environment.
            symbol_ptr_t new_sym = func_env.create_symbol(*arg_sym_iter);
            new_sym->set_value(env->eval(car_));

            ++arg_sym_iter;

            cdr_ = boost::dynamic_pointer_cast<cons_cell>(cdr_)->cdr();
        }

        object_ptr_t last_result = nil();
        cdr_ = nil();

        if(m_body)
            cdr_ = m_body;

        while(cdr_ != nil()) {
            object_ptr_t car_;

            if(cdr_->is_cons_cell())
                // Seems to be risky but it's not :-)
                car_ = boost::dynamic_pointer_cast<cons_cell>(cdr_)->car();
            else
                signal(env->get_symbol("wrong-type-argument"),
                       "listp " + cdr_->str());

            last_result = func_env.eval(car_);
            cdr_ = boost::dynamic_pointer_cast<cons_cell>(cdr_)->cdr();
        }

        return last_result;
    }

    std::string function::str() const
    {
        std::stringstream os;

        os << "#<function at " << this << ">";

        return os.str();
    }

    object_ptr_t defun_form::operator()(environment* env, const cons_cell_ptr_t args)
    {
        cons_cell_ptr_t cdr = boost::dynamic_pointer_cast<cons_cell>(args->cdr());
        object_ptr_t arg_list = cdr->car();

        if(!arg_list->is_cons_cell() && arg_list != nil())
            signal(env->get_symbol("wrong-type-argument"), "listp <argument-1 to defun>");

        function::arg_sym_list_t function_arg_list;
        cons_cell_ptr_t body = boost::dynamic_pointer_cast<cons_cell>(cdr->cdr());

        if(arg_list->is_cons_cell()) {
            cons_cell_ptr_t arg_list_cell = boost::dynamic_pointer_cast<cons_cell>(arg_list);

            while(arg_list_cell && *arg_list_cell) {
                if(arg_list_cell->car()->is_symbol_ref()) {
                    symbol_ref_ptr_t sym = boost::dynamic_pointer_cast<symbol_ref>(arg_list_cell->car());
                    function_arg_list.push_back(sym->name());

                    arg_list_cell = boost::dynamic_pointer_cast<cons_cell>(arg_list_cell->cdr());
                }
            }
        }

        return object_ptr_t(new function(function_arg_list, body));
    }
}
