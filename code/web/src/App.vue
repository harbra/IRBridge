<template>
  <div id="app" class="is-marginless">
    <router-view></router-view>
  </div>
</template>

<script>
import TopNav from './components/TopNav.vue'
import BottomFooter from './components/BottomFooter.vue'

export default {
  name: 'app',
  components: {
    TopNav,
    BottomFooter
  },
  data () {
    return {
    }
  }
}
</script>

<style>
@import '../node_modules/buefy/lib/buefy.css';
@import '../node_modules/bulma/css/bulma.css';
@import './assets/flipfont/style.css';

/* animation CSS classes borrowed from FontAwesome */
@-webkit-keyframes spin {
  0% {
    -webkit-transform: rotate(0deg);
    transform: rotate(0deg);
  }

  100% {
    -webkit-transform: rotate(359deg);
    transform: rotate(359deg);
  }
}

@keyframes spin {
  0% {
    -webkit-transform: rotate(0deg);
    transform: rotate(0deg);
  }

  100% {
    -webkit-transform: rotate(359deg);
    transform: rotate(359deg);
  }
}

.faa-spin.animated,
.faa-spin.animated-hover:hover,
.faa-parent.animated-hover:hover > .faa-spin {
  -webkit-animation: spin 1.5s linear infinite;
  animation: spin 1.5s linear infinite;
}

.faa-spin.animated.faa-fast,
.faa-spin.animated-hover.faa-fast:hover,
.faa-parent.animated-hover:hover > .faa-spin.faa-fast {
  -webkit-animation: spin 0.7s linear infinite;
  animation: spin 0.7s linear infinite;
}

.faa-spin.animated.faa-slow,
.faa-spin.animated-hover.faa-slow:hover,
.faa-parent.animated-hover:hover > .faa-spin.faa-slow {
  -webkit-animation: spin 2.2s linear infinite;
  animation: spin 2.2s linear infinite;
}

</style>
