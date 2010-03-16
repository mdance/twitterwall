// Create the tooltips only on document load
$(document).ready(function() 
{  
  var tweets = $('.tweet img');
  var tweetcount = tweets.size();
  
  if ( tweetcount > 0 ) {
    var style = {
      border: {
        width: 7,
        radius: 5
      },
      tip: 'topLeft',
      name: 'light'
    };
    
    // Uncomment for mouseover functionality
    $('.tweet img').each(function(){
      if( !$(this).attr('title') ) { 
        return; 
      }
    
      $(this).qtip({
        show: 'mouseover',
        hide: 'mouseout',
        style: style,                
        position: {
          adjust: {
            x: -15,
            y: -15
          }
        }
      });
    });
    
    var last = null;
    var random = null;
    
    var timer = setInterval(
      function() {
        random = Math.ceil(Math.random() * tweetcount);
  
        if ( last ) {
          $(tweets[last]).qtip("hide");
        }
        
        last = random;
        
        var tweet = $(tweets[random]);

        /*
        if( !tweet.attr('title') ) { 
          return;
        }
        */
        
        // Must use the alt text as the title attribute gets removed
        tweet.qtip({
          content: {
            text: tweet.attr('alt')
          },
          show: {
            ready: true
          },
          style: style, 
          position: {
            adjust: {
              x: -15,
              y: -15
            }
          }
        });
      },
      5000
    );
  }
});
