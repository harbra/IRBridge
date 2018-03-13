<template>
<div id="app" class="is-marginless">
  <TopNav product="Flipdisplay" title="Test" subtitle="This is for testing" menutitle="Menu" menulink="/">
  </TopNav>
  <section class="section">

  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import WebEvents from '../plugins/webevents'

import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'Test',
  components: {
  },
  data() {
    return {}
  },
  methods: {},
  created: function() {
    if (WebEvents.sourceAvailable()) {
      this.$eventsource.addEventListener('open', e => {
        //console.log("Events Connected");
      }, false);

      this.$eventsource.addEventListener('error', e => {
        if (e.target.readyState != EventSource.OPEN) {
          //console.log("Events Disconnected");
        }
      }, false);

      this.$eventsource.addEventListener('message', e => {
        console.log("message", e.data);
      }, false);

      this.$eventsource.addEventListener('test', e => {
        console.log("myevent", e.data);
      }, false);

      this.$eventsource.addEventListener('IRreceived', e => {
        console.log("myevent", JSON.parse(e.data));
      }, false);

      this.$eventsource.addEventListener('ConnectionCheck', e => {
        console.log("ConnectionCheck", JSON.parse(e.data));
        // this will be between 0% and 100%
        console.log("Signal quality:", Math.min(Math.max(2 * (JSON.parse(e.data).RSSI + 100), 0), 100));
      }, false);
    }
  }
}
</script>

<style scoped>

</style>
