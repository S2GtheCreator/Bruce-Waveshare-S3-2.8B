// SPDX-License-Identifier: AGPL-3.0-or-later
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
#include "nrf24_js.h"
#include "helpers_js.h"
JSValue native_nrf24Begin(JSContext *ctx, JSValue *, int, JSValue *) { return JS_ThrowInternalError(ctx, "NRF24 excluded: GPL-2.0-only backend"); }
JSValue native_nrf24Send(JSContext *ctx, JSValue *, int, JSValue *) { return JS_ThrowInternalError(ctx, "NRF24 excluded: GPL-2.0-only backend"); }
JSValue native_nrf24Receive(JSContext *ctx, JSValue *, int, JSValue *) { return JS_ThrowInternalError(ctx, "NRF24 excluded: GPL-2.0-only backend"); }
JSValue native_nrf24SetChannel(JSContext *ctx, JSValue *, int, JSValue *) { return JS_ThrowInternalError(ctx, "NRF24 excluded: GPL-2.0-only backend"); }
JSValue native_nrf24IsConnected(JSContext *ctx, JSValue *, int, JSValue *) { return JS_ThrowInternalError(ctx, "NRF24 excluded: GPL-2.0-only backend"); }
#endif
