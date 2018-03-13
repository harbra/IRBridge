/*jshint esversion: 6 */
// central configuration for server-sent events
export default {
  install(Vue, options) {
      if (typeof options !== 'object') {
          throw new Error("options is not an object");
      }

      if (!!window.EventSource) {
        Vue.prototype.$eventsource = new EventSource(options.eventsurl);
      }
    },
  sourceAvailable: function() {
    return (!!window.EventSource);
  }
}
