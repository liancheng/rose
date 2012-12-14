#include "utils.hpp"

#include "rose/eq.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/symbol.h"

class test_reader : public fixture_base {
protected:
    virtual void TearDown ()
    {
        r_reader_free (reader);
        fixture_base::TearDown ();
    }

    RDatumReader* set_input (rconstcstring input)
    {
        rsexp port;

        port   = r_open_input_string (state, r_string_new (state, input));
        reader = r_reader_new (state, port);

        return reader;
    }

    RDatumReader* reader;
};

TEST_F (test_reader, boolean)
{
    EXPECT_TRUE (r_eq_p (state, R_TRUE, r_read (set_input ("#t "))));
    EXPECT_TRUE (r_eq_p (state, R_FALSE, r_read (set_input ("#f "))));
}
