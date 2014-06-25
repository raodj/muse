

template<typename RetVal>
RSSAsyncHelper<RetVal>::RSSAsyncHelper() {
}

template<typename RetVal>
RSSAsyncHelper<RetVal>::RSSAsyncHelper(MethodCall method) : method(method) {

}

template<typename RetVal>
void
RSSAsyncHelper<RetVal>::run() {
   result = method();
}


template<typename RetVal>
RetVal
RSSAsyncHelper<RetVal>::getResult() {
    return result;
}

