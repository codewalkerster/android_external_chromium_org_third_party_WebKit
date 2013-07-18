/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
#include "core/dom/CustomElementCallbackInvocation.h"

#include "core/dom/CustomElementCallbackDispatcher.h"

namespace WebCore {

class CreatedInvocation : public CustomElementCallbackInvocation {
public:
    CreatedInvocation(PassRefPtr<CustomElementLifecycleCallbacks> callbacks)
        : CustomElementCallbackInvocation(callbacks)
    {
    }

private:
    virtual void dispatch(Element*) OVERRIDE;
};

void CreatedInvocation::dispatch(Element* element)
{
    if (element->inDocument())
        CustomElementCallbackDispatcher::instance().enqueueEnteredDocumentCallback(callbacks(), element);
    callbacks()->created(element);
}

class EnteredLeftDocumentInvocation : public CustomElementCallbackInvocation {
public:
    EnteredLeftDocumentInvocation(PassRefPtr<CustomElementLifecycleCallbacks>, CustomElementLifecycleCallbacks::CallbackType which);

private:
    virtual void dispatch(Element*) OVERRIDE;

    CustomElementLifecycleCallbacks::CallbackType m_which;
};

EnteredLeftDocumentInvocation::EnteredLeftDocumentInvocation(PassRefPtr<CustomElementLifecycleCallbacks> callbacks, CustomElementLifecycleCallbacks::CallbackType which)
    : CustomElementCallbackInvocation(callbacks)
    , m_which(which)
{
    ASSERT(m_which == CustomElementLifecycleCallbacks::EnteredDocument || m_which == CustomElementLifecycleCallbacks::LeftDocument);
}

void EnteredLeftDocumentInvocation::dispatch(Element* element)
{
    switch (m_which) {
    case CustomElementLifecycleCallbacks::EnteredDocument:
        callbacks()->enteredDocument(element);
        break;
    case CustomElementLifecycleCallbacks::LeftDocument:
        callbacks()->leftDocument(element);
        break;
    default:
        ASSERT_NOT_REACHED();
    }
}

class AttributeChangedInvocation : public CustomElementCallbackInvocation {
public:
    AttributeChangedInvocation(PassRefPtr<CustomElementLifecycleCallbacks>, const AtomicString& name, const AtomicString& oldValue, const AtomicString& newValue);

private:
    virtual void dispatch(Element*) OVERRIDE;

    AtomicString m_name;
    AtomicString m_oldValue;
    AtomicString m_newValue;
};

AttributeChangedInvocation::AttributeChangedInvocation(PassRefPtr<CustomElementLifecycleCallbacks> callbacks, const AtomicString& name, const AtomicString& oldValue, const AtomicString& newValue)
    : CustomElementCallbackInvocation(callbacks)
    , m_name(name)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
}

void AttributeChangedInvocation::dispatch(Element* element)
{
    callbacks()->attributeChanged(element, m_name, m_oldValue, m_newValue);
}

PassOwnPtr<CustomElementCallbackInvocation> CustomElementCallbackInvocation::createInvocation(PassRefPtr<CustomElementLifecycleCallbacks> callbacks, CustomElementLifecycleCallbacks::CallbackType which)
{
    switch (which) {
    case CustomElementLifecycleCallbacks::Created:
        return adoptPtr(new CreatedInvocation(callbacks));

    case CustomElementLifecycleCallbacks::EnteredDocument:
    case CustomElementLifecycleCallbacks::LeftDocument:
        return adoptPtr(new EnteredLeftDocumentInvocation(callbacks, which));

    default:
        ASSERT_NOT_REACHED();
        return PassOwnPtr<CustomElementCallbackInvocation>();
    }
}

PassOwnPtr<CustomElementCallbackInvocation> CustomElementCallbackInvocation::createAttributeChangedInvocation(PassRefPtr<CustomElementLifecycleCallbacks> callbacks, const AtomicString& name, const AtomicString& oldValue, const AtomicString& newValue)
{
    return adoptPtr(new AttributeChangedInvocation(callbacks, name, oldValue, newValue));
}

} // namespace WebCore