/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "V8PopStateEvent.h"

#include "V8History.h"
#include "bindings/v8/SerializedScriptValue.h"
#include "bindings/v8/V8HiddenPropertyName.h"
#include "core/dom/PopStateEvent.h"
#include "core/page/History.h"

namespace WebCore {

// Save the state value to a hidden attribute in the V8PopStateEvent, and return it, for convenience.
static v8::Handle<v8::Value> cacheState(v8::Handle<v8::Object> popStateEvent, v8::Handle<v8::Value> state)
{
    popStateEvent->SetHiddenValue(V8HiddenPropertyName::state(), state);
    return state;
}

void V8PopStateEvent::stateAttrGetterCustom(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Value> result = info.Holder()->GetHiddenValue(V8HiddenPropertyName::state());

    if (!result.IsEmpty()) {
        v8SetReturnValue(info, result);
        return;
    }

    PopStateEvent* event = V8PopStateEvent::toNative(info.Holder());
    if (!event->state().hasNoValue()) {
        v8SetReturnValue(info, cacheState(info.Holder(), event->state().v8Value()));
        return;
    }

    History* history = event->history();
    if (!history || !event->serializedState()) {
        v8SetReturnValue(info, cacheState(info.Holder(), v8::Null(info.GetIsolate())));
        return;
    }

    // There's no cached value from a previous invocation, nor a state value was provided by the
    // event, but there is a history object, so first we need to see if the state object has been
    // deserialized through the history object already.
    // The current history state object might've changed in the meantime, so we need to take care
    // of using the correct one, and always share the same deserialization with history.state.

    bool isSameState = history->isSameAsCurrentState(event->serializedState().get());

    if (isSameState) {
        v8::Handle<v8::Object> v8History = toV8Fast(history, info, event).As<v8::Object>();
        if (!history->stateChanged()) {
            result = v8History->GetHiddenValue(V8HiddenPropertyName::state());
            if (!result.IsEmpty()) {
                v8SetReturnValue(info, cacheState(info.Holder(), result));
                return;
            }
        }
        result = event->serializedState()->deserialize(info.GetIsolate());
        v8History->SetHiddenValue(V8HiddenPropertyName::state(), result);
    } else
        result = event->serializedState()->deserialize(info.GetIsolate());

    v8SetReturnValue(info, cacheState(info.Holder(), result));
}

} // namespace WebCore