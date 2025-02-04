#include "ut/TagProcessorBase.h"
#include "ut/CharWriter.h"

namespace nw4r {
    namespace ut {
        template <typename CharType>
        TagProcessorBase<CharType>::TagProcessorBase() {
            
        }

        template<typename CharType>
        TagProcessorBase<CharType>::~TagProcessorBase() {

        }

        template<typename CharType>
        void TagProcessorBase<CharType>::ProcessLinefeed(ContextType *context) {
            TextWriterBase<CharType>& writer = *context->writer;
            const f32 x = context->xOrigin;
            const f32 y = writer.GetCursorY() + writer.GetLineHeight();
            writer.SetCursor(x, y);
        }
    };
};
