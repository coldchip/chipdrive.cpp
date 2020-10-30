const HtmlWebpackPlugin = require('html-webpack-plugin');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const CopyPlugin = require('copy-webpack-plugin');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');

module.exports = {
  mode: 'production',
  entry: './js/index.js',
  output: {
    path: __dirname + '/bin/',
    filename: '[name].[contenthash].js',
    chunkFilename: '[id].chunk.[contenthash].js'
  },
  optimization: {
      splitChunks: {
        chunks: 'all',
      }
  },
  module: {
    rules: [
      {
        test: /\.css$/i,
        use: [MiniCssExtractPlugin.loader, 'css-loader'],
      },
      {
        test: /\.(woff|ttf|otf|eot|woff2|svg)$/i,
        loader: "file-loader?name=font.[contenthash].[ext]"
      }
    ]
  },
  plugins: [
    new CleanWebpackPlugin(),
  	new MiniCssExtractPlugin({
      filename: '[name].[contenthash].css',
      chunkFilename: '[id].chunk.[contenthash].css',
    }),
    new HtmlWebpackPlugin({
      template: './index.html'
    }),
    new CopyPlugin({
      patterns:[
        { from: 'img', to: 'img'},
        { from: 'template', to: 'template'}
      ]
    })
  ]
}